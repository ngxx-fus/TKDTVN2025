#include "../../include/projectConfig.h"
#if (LAN_DATA_EXCHANGE_EN == 1)

#include "espLANDataExchange.h"

WiFiUDP eldeUdp;
uint8_t eldeUdpRxBuf[ELDE_RX_BUF_SIZE];

/// Variable to store the length of the last received packet
static int _lastRxLen = 0;

/// Static buffer for Self IP
static char _selfIpStrBuf[16] = "0.0.0.0";

espLANHost_t thisESP = {
    .ip = _selfIpStrBuf,
    .hostName = "ESP32_CLIENT",
    .port = { .udp = THIS_ESP_UDP_PORT, .tcp = THIS_ESP_TCP_PORT }
};

espLANHost_t broadcastHost = {
    .ip = "255.255.255.255", // Broadcast or specific IP
    .hostName = "CENTER_CONTROL_SERVICE",
    .port = { .udp = SERVER_UDP_PORT, .tcp = SERVER_TCP_PORT }
};

/// @brief Update thisESP.ip with current WiFi local IP
void eldeUpdateSelfIP() {
    if (WiFi.status() == WL_CONNECTED) {
        String ip = WiFi.localIP().toString();
        strncpy(_selfIpStrBuf, ip.c_str(), sizeof(_selfIpStrBuf) - 1);
    }
}

def eldeInit() {
    if (WiFi.status() != WL_CONNECTED) return STATUS_ERR;
    eldeUpdateSelfIP();
    if (eldeUdp.begin(thisESP.port.udp)) {
        __sys_log("[ELDE] Listening on UDP Port: %d", thisESP.port.udp);
        return STATUS_OKE;
    }
    return STATUS_ERR;
}

/// @brief Calculate CRC16 (Poly 0x1021) supporting both HW and SW
uint16_t computeCRC16(void* ByteArr, int ByteArrSize, int16_t CRCInitValue) {
    uint8_t* data = (uint8_t*)ByteArr;
    
    #if (defined(ESP32) && (ESP_USE_CRC_HW == 1))
        /// [HW] Use ESP32 ROM CRC (Fast, Big Endian result matching Poly 0x1021)
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

/// @brief Construct and Send Frame
def eldeSendFrame(uint8_t id, void* data, uint8_t len, const espLANHost_t* target) {
    if (target == NULL) return STATUS_ERR;

    uint8_t txBuf[256];
    int idx = 0;
    
    /// 1. Frame Start: "FRAME_BEGIN"
    size_t tagStartLen = strlen(ELDE_FRAME_BEGIN);
    memcpy(&txBuf[idx], ELDE_FRAME_BEGIN, tagStartLen);
    idx += tagStartLen;

    /// --- PAYLOAD BEGIN (CRC Calculation Start) ---
    int payloadStartIdx = idx;

    /// 2. Payload: <ID>
    txBuf[idx++] = id;

    /// 3. Payload: <Data>
    if (len > 0 && data != NULL) {
        memcpy(&txBuf[idx], data, len);
        idx += len;
    }
    
    /// Determine Payload Size
    int payloadSize = idx - payloadStartIdx;
    
    /// 4. Calculate CRC on Payload ONLY (ID + Data)
    uint16_t crcVal = computeCRC16(&txBuf[payloadStartIdx], payloadSize, 0xFFFF);

    /// 5. Tag: "CRC"
    size_t tagCrcLen = strlen(ELDE_TAG_CRC);
    memcpy(&txBuf[idx], ELDE_TAG_CRC, tagCrcLen);
    idx += tagCrcLen;

    /// 6. CRC Value: <HighByte><LowByte>
    txBuf[idx++] = (uint8_t)((crcVal >> 8) & 0xFF); /// High Byte
    txBuf[idx++] = (uint8_t)(crcVal & 0xFF);        /// Low Byte

    /// 7. Frame End: "FRAME_END"
    size_t tagEndLen = strlen(ELDE_FRAME_END);
    memcpy(&txBuf[idx], ELDE_FRAME_END, tagEndLen);
    idx += tagEndLen;

    /// 8. Send UDP
    if (WiFi.status() == WL_CONNECTED && eldeUdp.beginPacket(target->ip, target->port.udp)) {
        eldeUdp.write(txBuf, idx);
        if (eldeUdp.endPacket()) return STATUS_OKE;
    }
    
    return STATUS_ERR;
}

/// @brief Check UDP buffer, skip loopbacks, read first valid packet within timeout
/// @param timeoutMs Maximum time (in ms) to spend processing the buffer to avoid blocking
/// @return 1 (STATUS_OKE) if valid packet found, 0 (STATUS_ERR) on timeout or empty buffer
def hasReceivedFrame(uint32_t timeoutMs) {
    /// Convert timeout to microseconds (us) for internal precision
    int64_t timeoutUs = (int64_t)timeoutMs * 1000;
    
    /// Get start time in microseconds (uptime since boot)
    int64_t startUs = esp_timer_get_time();
    
    int packetSize;
    
    /// Loop until buffer is empty or valid packet found
    while ((packetSize = eldeUdp.parsePacket())) {
        
        /// 0. Yield to OS: Prevent WiFi stack starvation & watchdog issues
        yield(); 

        /// 1. Timeout Protection (Compare in microseconds)
        if ((esp_timer_get_time() - startUs) > timeoutUs) {
            return 0; /// Timeout occurred
        }

        /// 2. Filter Loopback: Ignore packets from self
        if (eldeUdp.remoteIP() == WiFi.localIP()) {
            /// Trash packet (Important: Flush/Read to remove it from stack)
            eldeUdp.flush(); 
            continue; 
        }

        /// 3. Valid packet found!
        /// Cap packet size to prevent buffer overflow
        if (packetSize > ELDE_RX_BUF_SIZE) {
             packetSize = ELDE_RX_BUF_SIZE; 
        }

        /// Read payload into global buffer
        int len = eldeUdp.read(eldeUdpRxBuf, packetSize);
        if (len > 0) {
            _lastRxLen = len;
            return 1; /// STATUS_OKE
        }
    }
    
    return 0; /// STATUS_ERR (No packet or Empty)
}

/// @brief Parse Frame, Validate CRC, Extract Data
def getFrameData(uint8_t *ID, int *DataSize, void* Buffer, int BufferSize) {
    if (_lastRxLen == 0) return STATUS_ERR;

    /// 1. Calculate structure sizes
    size_t startTagLen = strlen(ELDE_FRAME_BEGIN);
    size_t endTagLen = strlen(ELDE_FRAME_END);
    size_t crcTagLen = strlen(ELDE_TAG_CRC);
    size_t crcValLen = 2;

    /// Minimal size = START + ID(1) + CRC_TAG + CRC_VAL + END
    size_t minLen = startTagLen + 1 + crcTagLen + crcValLen + endTagLen;

    if (_lastRxLen < minLen) {
        _lastRxLen = 0; /// Clear flag
        return STATUS_ERR;
    }

    /// 2. Validate Header (FRAME_BEGIN)
    if (memcmp(eldeUdpRxBuf, ELDE_FRAME_BEGIN, startTagLen) != 0) {
        return STATUS_ERR;
    }

    /// 3. Validate Footer (FRAME_END) - At the very end
    int endTagIdx = _lastRxLen - endTagLen;
    if (memcmp(&eldeUdpRxBuf[endTagIdx], ELDE_FRAME_END, endTagLen) != 0) {
        return STATUS_ERR;
    }

    /// 4. Validate CRC Tag ("CRC") - Before CRC Value
    int crcValIdx = endTagIdx - crcValLen;
    int crcTagIdx = crcValIdx - crcTagLen;
    if (memcmp(&eldeUdpRxBuf[crcTagIdx], ELDE_TAG_CRC, crcTagLen) != 0) {
        return STATUS_ERR;
    }

    /// 5. Extract CRC from frame (Little Endian)
    uint16_t receivedCRC = (uint16_t)eldeUdpRxBuf[crcValIdx] << 8 | eldeUdpRxBuf[crcValIdx + 1];

    /// 6. Calculate CRC for Payload (From ID up to CRC Tag Start)
    /// Payload starts after FRAME_BEGIN, ends at CRC Tag
    int payloadStartIdx = startTagLen;
    int payloadLen = crcTagIdx - payloadStartIdx;
    
    uint16_t calculatedCRC = computeCRC16(&eldeUdpRxBuf[payloadStartIdx], payloadLen, 0xFFFF);

    if (calculatedCRC != receivedCRC) {
        __sys_err("[ELDE] CRC Mismatch! RX:0x%04X Calc:0x%04X", receivedCRC, calculatedCRC);
        return STATUS_ERR;
    }

    /// 7. CRC OK - Extract Data
    /// ID is the first byte of payload
    if (ID != NULL) {
        *ID = eldeUdpRxBuf[payloadStartIdx];
    }

    /// Data is remaining bytes of payload
    int dataLen = payloadLen - 1; // Subtract ID byte
    if (DataSize != NULL) {
        *DataSize = dataLen;
    }

    /// Copy data to user buffer if valid
    if (dataLen > 0) {
        if (dataLen > BufferSize) {
             __sys_err("[ELDE] User buffer too small! Need: %d", dataLen);
             return STATUS_ERR;
        }

        /// Skip memcpy if user passed the system buffer itself
        if (Buffer != NULL && Buffer != eldeUdpRxBuf) {
            memcpy(Buffer, &eldeUdpRxBuf[payloadStartIdx + 1], dataLen);
        }
    }

    /// Reset length to avoid re-reading same packet
    _lastRxLen = 0; 
    
    return STATUS_OKE;
}

/// @brief Poll for incoming UDP packets
void eldePoll() {
    /// Simply check and discard if just polling without processing
    /// Or use hasReceivedFrame() in main loop
    if (hasReceivedFrame(1)) {
        // Auto clear if not processed immediately in Poll mode
        // Or keep it for getFrameData
    }
}

#endif