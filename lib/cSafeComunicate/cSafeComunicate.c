#include "cSafeComunicate.h"
#include <stdio.h>

// --- CONSTANTS DEFINITION ---

// Hex: 46 52 41 4d 45 5f 42 45 47 49 4e -> "FRAME_BEGIN"
static const uint8_t FRAME_BEGIN[] = {0x46, 0x52, 0x41, 0x4D, 0x45, 0x5F, 0x42, 0x45, 0x47, 0x49, 0x4E};
// Hex: 46 52 41 4d 45 5f 45 4e 44       -> "FRAME_END"
static const uint8_t FRAME_END[]   = {0x46, 0x52, 0x41, 0x4D, 0x45, 0x5F, 0x45, 0x4E, 0x44};
// Hex: 43 52 43                         -> "CRC"
static const uint8_t CRC_MARKER[]  = {0x43, 0x52, 0x43};

#define LEN_BEGIN   (sizeof(FRAME_BEGIN)) // 11
#define LEN_END     (sizeof(FRAME_END))   // 9
#define LEN_CRC_MK  (sizeof(CRC_MARKER))  // 3
#define LEN_CRC_VAL 2                     // 2 bytes for CRC16
#define LEN_ID      1                     // 1 byte for ID

// Total overhead = BEGIN(11) + ID(1) + CRC_MK(3) + CRC_VAL(2) + END(9) = 26 bytes (Actually ID is payload context but part of structure)
#define MIN_FRAME_SIZE (LEN_BEGIN + LEN_ID + LEN_CRC_MK + LEN_CRC_VAL + LEN_END)

// --- INTERNAL BUFFERS FOR POLLING ---
static uint8_t buffer[CPACKAGING_SIZE];
static int bufIdx = 0;

/// @brief FSM States for Polling
typedef enum {
    STATE_WAIT_HEADER,
    STATE_READ_PAYLOAD
} PollState_t;

static PollState_t pollState = STATE_WAIT_HEADER;
static int headerMatchIdx = 0;


// --- CRC IMPLEMENTATION ---

/// @brief Calculate CRC16 (Poly 0x1021) supporting both HW and SW
uint16_t computeCRC16(void* ByteArr, int ByteArrSize, int16_t CRCInitValue) {
    uint8_t* data = (uint8_t*)ByteArr;
    
    #if (defined(ESP32) && (ESP_USE_CRC_HW == 1))
        /// [HW] Use ESP32 ROM CRC (Fast, Big Endian result matching Poly 0x1021)
        // Note: Make sure esp_rom_crc.h is included if compiling on ESP-IDF
        // #include "esp_rom_crc.h" 
        return esp_rom_crc16_be((uint16_t)CRCInitValue, data, (uint32_t)ByteArrSize);
    #else
        /// [SW] Manual calculation for portability
        uint16_t crc = (uint16_t)CRCInitValue;
        for (int i = 0; i < ByteArrSize; i++) {
            crc ^= ((uint16_t)data[i] << 8);
            for (uint8_t j = 0; j < 8; j++) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ CRC_POLYNOMIAL;
                } else {
                    crc <<= 1;
                }
            }
        }
        return crc;
    #endif
}

// --- MAIN FUNCTIONS ---

def DataFrameEncode(uint8_t ID, void* ByteArr, int ByteArrSize, void* PackedByteArr, int PackedByteArrMaxSize) {
    int totalSize = LEN_BEGIN + LEN_ID + ByteArrSize + LEN_CRC_MK + LEN_CRC_VAL + LEN_END;

    if (totalSize > PackedByteArrMaxSize) {
        return STATUS_ERR; // Buffer overflow
    }

    uint8_t* pDest = (uint8_t*)PackedByteArr;
    int cursor = 0;

    // 1. Add FRAME_BEGIN
    memcpy(&pDest[cursor], FRAME_BEGIN, LEN_BEGIN);
    cursor += LEN_BEGIN;

    // 2. Add ID
    pDest[cursor++] = ID;

    // 3. Add Data
    if (ByteArrSize > 0 && ByteArr != NULL) {
        memcpy(&pDest[cursor], ByteArr, ByteArrSize);
        cursor += ByteArrSize;
    }

    // 4. Calculate CRC16 (Only include Payload: ID + Data)
    // Create a temporary buffer or calc in parts?
    // CRC is calculated on [ID] + [Data]
    uint16_t crcVal = 0;
    // Calc CRC for ID
    crcVal = computeCRC16(&ID, 1, 0); 
    // Continue Calc CRC for Data (using previous crc as init)
    if (ByteArrSize > 0) {
        crcVal = computeCRC16(ByteArr, ByteArrSize, crcVal);
    }

    // 5. Add CRC Marker
    memcpy(&pDest[cursor], CRC_MARKER, LEN_CRC_MK);
    cursor += LEN_CRC_MK;

    // 6. Add CRC16 Value (High Byte then Low Byte)
    pDest[cursor++] = (uint8_t)((crcVal >> 8) & 0xFF);
    pDest[cursor++] = (uint8_t)(crcVal & 0xFF);

    // 7. Add FRAME_END
    memcpy(&pDest[cursor], FRAME_END, LEN_END);
    cursor += LEN_END;

    return cursor; // Return total packed size
}

def DataFrameDecode(void* PackedByteArr, int PackedByteArrSize, uint8_t *ID, void* ByteArr, int ByteArrMaxSize) {
    uint8_t* pSrc = (uint8_t*)PackedByteArr;

    // 1. Check Minimum Size
    if (PackedByteArrSize < MIN_FRAME_SIZE) {
        return STATUS_ERR;
    }

    // 2. Check FRAME_BEGIN
    if (memcmp(pSrc, FRAME_BEGIN, LEN_BEGIN) != 0) {
        return STATUS_ERR;
    }

    // 3. Check FRAME_END (At the end of the buffer)
    if (memcmp(&pSrc[PackedByteArrSize - LEN_END], FRAME_END, LEN_END) != 0) {
        return STATUS_ERR;
    }

    // 4. Locate CRC Marker and Values
    // Structure: [BEGIN] [ID] [DATA...] [CRC_MK] [CRC_H] [CRC_L] [END]
    // The CRC section (Marker + 2 bytes val) ends right before FRAME_END
    int crcSectionEndIndex = PackedByteArrSize - LEN_END;
    int crcMarkerIndex = crcSectionEndIndex - LEN_CRC_VAL - LEN_CRC_MK;

    // Verify CRC Marker
    if (memcmp(&pSrc[crcMarkerIndex], CRC_MARKER, LEN_CRC_MK) != 0) {
        return STATUS_ERR;
    }

    // 5. Extract CRC Value from Frame
    uint16_t receivedCRC = ((uint16_t)pSrc[crcMarkerIndex + LEN_CRC_MK] << 8) | 
                            (uint16_t)pSrc[crcMarkerIndex + LEN_CRC_MK + 1];

    // 6. Calculate CRC on Payload (ID + Data)
    // Payload starts after BEGIN (index LEN_BEGIN)
    // Payload ends before CRC Marker (index crcMarkerIndex)
    int payloadStartIndex = LEN_BEGIN;
    int payloadLen = crcMarkerIndex - payloadStartIndex;

    if (payloadLen < 1) { // Must have at least ID
        return STATUS_ERR;
    }

    uint16_t calcCRC = computeCRC16(&pSrc[payloadStartIndex], payloadLen, 0);

    // 7. Verify CRC
    if (calcCRC != receivedCRC) {
        return STATUS_ERR; // Corrupted
    }

    // 8. Extract ID
    *ID = pSrc[payloadStartIndex];

    // 9. Extract Data
    int dataLen = payloadLen - 1; // Subtract 1 for ID
    if (dataLen > ByteArrMaxSize) {
        return STATUS_ERR; // Output buffer too small
    }

    if (dataLen > 0) {
        memcpy(ByteArr, &pSrc[payloadStartIndex + 1], dataLen);
    }

    return dataLen; // Return extracted data size
}

def DataFramePoll(uint8_t NewByte) {
    switch (pollState) {
        case STATE_WAIT_HEADER:
            // Check byte against FRAME_BEGIN sequence
            if (NewByte == FRAME_BEGIN[headerMatchIdx]) {
                headerMatchIdx++;
                if (headerMatchIdx >= LEN_BEGIN) {
                    // Header found completely
                    bufIdx = 0;
                    // Copy header to buffer to keep the frame complete for Decode function
                    memcpy(buffer, FRAME_BEGIN, LEN_BEGIN);
                    bufIdx = LEN_BEGIN;
                    
                    pollState = STATE_READ_PAYLOAD;
                }
            } else {
                // Mismatch, reset search. 
                // Optimization: Could re-check if NewByte matches FRAME_BEGIN[0] but for simplicity reset to 0
                if (NewByte == FRAME_BEGIN[0]) {
                    headerMatchIdx = 1;
                } else {
                    headerMatchIdx = 0;
                }
            }
            break;

        case STATE_READ_PAYLOAD:
            if (bufIdx >= CPACKAGING_SIZE) {
                // Buffer overflow, discard and reset
                pollState = STATE_WAIT_HEADER;
                headerMatchIdx = 0;
                return STATUS_ERR;
            }

            buffer[bufIdx++] = NewByte;

            // Check if the tail matches FRAME_END
            // We need enough bytes for [BEGIN]...[END] at least
            if (bufIdx >= MIN_FRAME_SIZE) {
                // Check last 'LEN_END' bytes
                if (memcmp(&buffer[bufIdx - LEN_END], FRAME_END, LEN_END) == 0) {
                    // FRAME_END detected. Now validate the whole frame (CRC check included in Decode)
                    // We treat 'buffer' as PackedByteArr
                    
                    // Note: In a real app, you might want to call Decode here or flag it ready.
                    // For this request, we perform a validation check.
                    
                    uint8_t dummyID;
                    // To avoid large stack usage, we can't allocate a huge buffer here, 
                    // but Decode requires an output buffer. 
                    // Since Poll is just checking reception status, we assume the user will 
                    // access the valid frame in 'buffer' or we verify CRC manually.
                    // Let's re-use the Decode function to verify validity strictly.
                    
                    // Use a small scratch buffer or pointer math since we just want to validate
                    // However, we strictly need to return STATUS_OKE only if CRC passes.
                    
                    // Let's implement a "Check Only" mode or just manual check here to be fast
                    // Re-calculate CRC logic from Decode:
                    
                    int crcSectionEndIndex = bufIdx - LEN_END;
                    int crcMarkerIndex = crcSectionEndIndex - LEN_CRC_VAL - LEN_CRC_MK;
                    
                    // Check CRC Marker existence
                    if (memcmp(&buffer[crcMarkerIndex], CRC_MARKER, LEN_CRC_MK) == 0) {
                         uint16_t rxCRC = ((uint16_t)buffer[crcMarkerIndex + LEN_CRC_MK] << 8) | 
                                          (uint16_t)buffer[crcMarkerIndex + LEN_CRC_MK + 1];
                         
                         int payloadStart = LEN_BEGIN;
                         int payloadLen = crcMarkerIndex - payloadStart; // ID + Data
                         
                         uint16_t calcCRC = computeCRC16(&buffer[payloadStart], payloadLen, 0);
                         
                         if (calcCRC == rxCRC) {
                             // Success! Frame received and verified.
                             // Reset state for next frame
                             pollState = STATE_WAIT_HEADER;
                             headerMatchIdx = 0;
                             return STATUS_OKE; 
                         }
                    }
                    
                    // If we reached here, FRAME_END matched but CRC/Marker failed.
                    // It could be that FRAME_END bytes appeared in data payload.
                    // We continue reading... unless buffer is full.
                }
            }
            break;
    }

    return STATUS_ERR; // Not yet finished or error
}