#ifndef __SAFE_UART_COMMUNICATE_H__
#define __SAFE_UART_COMMUNICATE_H__

/// EXTEND FOR C++ //////////////////////////////////////////////////
#ifdef __cplusplus                                                ///
extern "C" {                                                      ///
#endif                                                            ///
/////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

/// Local util: __isnot_null, ...
#include "../cArithmeticMacro/cArithmeticMacro.h"
/// Local util: __sys_log, ...
#include "../espSerialWrap/espSerialWrap.h"
/// Local util: def = int, predefine ERROR CODE
#include "../cReturnType/cReturnType.h"

/// Note: sdf = safe data frame

#define __START_FRAME__     0x00
#define __END_FRAME__       0xFF
#define __BREAK_FRAME__     __END_FRAME__
#define __sdf_malloc        malloc

#define __sdf_log           __sys_log
#define __sdf_err           __sys_err
#define __sdf_entry         __entry
#define __sdf_exit          __exit

#define __sdfGeneratorPolynomial 0x1021 

#ifndef __SAFE_DATAFRAME_MUTEX__
    #define __SAFE_DATAFRAME_MUTEX__ 0
#endif

#define __SDF_IF_NULL(ptr, ...) \
    do { \
        if ((ptr) == NULL) { \
            __VA_ARGS__; \
        } \
    } while (0)


#define __SDF_MALLOC(type, size, ...)                              \
    ({                                                              \
        type* __ptr = (type*)malloc((size));         \
        if (__ptr == NULL) {                                        \
            __VA_ARGS__;                                            \
        } else {                                                    \
            memset(__ptr, 0, (size));                \
        }                                                           \
        __ptr;                                                      \
    })

typedef struct safeTransferContext_t{
    union {
        uint8_t     __1st_byte;
        struct {
            uint8_t id  : 4;
            uint8_t len : 4;
        };  
    };
    uint8_t*        buffPtr;                     /// Buffer for send/received
    uint16_t        buffSize;                    /// Buffer size
    uint16_t        buffBitInd;                  /// Bit index in buffer

    /// Only use via specific features; do not access directly
    uint16_t        __crc;
    uint8_t         __continuousLogicCount;

    /// Local informations for API
    uint8_t         __isReceiving;
    int64_t         __lastReceivedTimeUS;

    /// For mutex in multi-task
    #if (__SAFE_DATAFRAME_MUTEX__ == 1)
        /// Reserved
    #endif
} safeTransferContext_t;

#ifdef ESP_PLATFORM

    #include "esp_rom_crc.h" /// Required for ESP ROM functions

    /// Computes CRC-16-CCITT (BE) using ESP ROM.
    static inline uint16_t __sdfCRCCompute(uint16_t init_crc, const void* data, int16_t size) {
        /// Validate inputs
        if (__is_null(data) || __isnot_positive(size)) {
            return init_crc; /// Return initial value on error
        }
        
        /// Use ESP ROM CRC (Big-Endian, 0x1021)
        return esp_rom_crc16_be(init_crc, (const uint8_t*)data, size);
    }

    /// Checks CRC-16 of a buffer (CRC at the end).
    static inline def __sdfCRCCheck(const void* dataWithCrc, int16_t sizeWithCrc) {
        /// Validate inputs
        if (__is_null(dataWithCrc)) {
            __sdf_err("CRC check data is null!"); 
            return STATUS_ERR;
        }
        
        /// Must have at least 2 bytes for CRC
        if (sizeWithCrc <= 2) { 
            __sdf_err("CRC check size (%d) is too small!", sizeWithCrc);
            return STATUS_ERR;
        }

        /// Separate data and CRC
        const uint8_t* payload_ptr = (const uint8_t*) dataWithCrc;
        int16_t payload_size = sizeWithCrc - 2;
        
        /// Extract received CRC (Big-Endian)
        uint16_t received_crc = ((uint16_t)payload_ptr[payload_size] << 8) | 
                                ((uint16_t)payload_ptr[payload_size + 1]);

        /// Compute CRC on the payload only
        /// Must use the same initial value (0)
        uint16_t computed_crc = __sdfCRCCompute(0, payload_ptr, payload_size);

        /// Compare
        if (computed_crc == received_crc) {
            return OK; /// Match!
        }
        
        /// CRC Mismatch
        __sdf_err("CRC mismatch! Computed: 0x%04X, Received: 0x%04X", computed_crc, received_crc);
        return STATUS_ERR;
    }
#else /// Host (Linux/PC)

    /// Computes CRC-16-CCITT (BE) using software.
    static inline uint16_t __sdfCRCCompute(uint16_t init_crc, const void* data, int16_t size) {
        /// Validate inputs
        if (__is_null(data) || __isnot_positive(size)) {
            return init_crc; /// Return initial value on error
        }

        /// Software calculation
        const uint8_t* ptr = (const uint8_t*)data;
        uint16_t crcResult = init_crc; /// Use initial value

        for (int16_t i = 0; i < size; i++) {
            crcResult ^= ((uint16_t)ptr[i] << 8); /// XOR byte in (Big-Endian)
            for (uint8_t j = 0; j < 8; j++) {
                if (crcResult & 0x8000) {
                    crcResult = (crcResult << 1) ^ __sdfGeneratorPolynomial;
                } else {
                    crcResult <<= 1;
                }
            }
        }
        return crcResult;
    }

    /// Checks CRC-16 of a buffer (CRC at the end).
    static inline def __sdfCRCCheck(const void* dataWithCrc, int16_t sizeWithCrc) {
        /// Validate inputs
        if (__is_null(dataWithCrc)) {
            __sdf_err("CRC check data is null!"); 
            return STATUS_ERR;
        }
        
        /// Must have at least 2 bytes for CRC
        if (sizeWithCrc <= 2) { 
            __sdf_err("CRC check size (%d) is too small!", sizeWithCrc);
            return STATUS_ERR;
        }

        /// Separate data and CRC
        const uint8_t* payload_ptr = (const uint8_t*) dataWithCrc;
        int16_t payload_size = sizeWithCrc - 2;
        
        /// Extract received CRC (Big-Endian)
        uint16_t received_crc = ((uint16_t)payload_ptr[payload_size] << 8) | 
                                ((uint16_t)payload_ptr[payload_size + 1]);

        /// Compute CRC on the payload only
        uint16_t computed_crc = __sdfCRCCompute(0, payload_ptr, payload_size);

        /// Compare
        if (computed_crc == received_crc) {
            return OK; /// Match!
        }
        
        /// CRC Mismatch
        __sdf_err("CRC mismatch! Computed: 0x%04X, Received: 0x%04X", computed_crc, received_crc);
        return STATUS_ERR;
    }
#endif

safeTransferContext_t* sdfInit(void* buffPtr, int16_t buffSize){
    __sdf_entry("sdfInit(%p, %d)", buffPtr, buffSize);
    if(__isnot_positive(buffSize)){
        __sdf_err("[sdfInit] Wrong buffer size!");
        /// Size Error
        return NULL;
    }
    /// Create safeTransferContext_t oject
    safeTransferContext_t* sdfContext = __SDF_MALLOC(safeTransferContext_t, sizeof(safeTransferContext_t), 
        __sdf_err("[sdfInit] Allocate safeTransferContext_t object failed!"); 
        return  NULL;
    );
    /// Check if buffPtr is null or not? If [null] ---> allocate; else assign from buffPtr to sdfContext->buffPtr
    if(__is_null(buffPtr)){
        /// allocate in this function
        sdfContext->buffPtr = __SDF_MALLOC(uint8_t, sizeof(uint8_t) * buffSize, 
            __sdf_err("[sdfInit] Allocate buffer failed!"); 
            return  NULL;
        );
    }else{
        sdfContext->buffPtr = (uint8_t*) buffPtr;
    }
    /// Assign sdfContext->buffSize 
    sdfContext->buffSize      = buffSize;
    /// Reset others
    sdfContext->__1st_byte    = 0x0;
    sdfContext->__crc         = 0x0;
    sdfContext->__continuousLogicCount = 0;
    sdfContext->__isReceiving = 0;
    sdfContext->__lastReceivedTimeUS = 0;

    return sdfContext;
}

static inline def sdfEncode(safeTransferContext_t* sdfContext, const void* data, int16_t size) {
    __sdf_entry("sdfEncode(ctx, data, %d)", size);

    /// 1. Validate inputs
    __SDF_IF_NULL(sdfContext, __sdf_err("Context is null!"); return STATUS_ERR;);
    __SDF_IF_NULL(sdfContext->buffPtr, __sdf_err("Context buffer is null!"); return STATUS_ERR;);
    __SDF_IF_NULL(data, __sdf_err("Input data is null!"); return STATUS_ERR;);

    /// Check payload size (4-bit length field)
    if (size <= 0 || size > 15) {
        __sdf_err("Invalid payload size: %d (must be 1-15)", size);
        return STATUS_ERR;
    }

    /// 2. Check buffer capacity
    /// Frame: START(1) + HDR(1) + PAYLOAD(size) + CRC(2) + END(1)
    int16_t totalFrameSize = size + 5;
    if (totalFrameSize > sdfContext->buffSize) {
        __sdf_err("Buffer size %d too small for frame size %d", sdfContext->buffSize, totalFrameSize);
        return STATUS_ERR;
    }

    /// 3. Get buffer pointer
    uint8_t* p = sdfContext->buffPtr;

    /// 4. Write Start Byte
    *p++ = __START_FRAME__;

    /// 5. Write Header (ID is assumed to be pre-set in sdfContext)
    sdfContext->len = size; /// Set length field
    uint8_t header = sdfContext->__1st_byte; /// Get combined (ID | LEN)
    *p++ = header;

    /// 6. Write Payload
    memcpy(p, data, size);
    p += size;

    /// 7. Compute and Write CRC
    /// CRC is computed over [HEADER] + [PAYLOAD]
    uint16_t crc = __sdfCRCCompute(0, &sdfContext->buffPtr[1], size + 1);
    *p++ = (uint8_t)(crc >> 8); /// CRC High-Byte (Big-Endian)
    *p++ = (uint8_t)(crc & 0xFF); /// CRC Low-Byte

    /// 8. Write End Byte
    *p++ = __END_FRAME__;

    /// Store total bytes written (optional, but good practice)
    sdfContext->buffBitInd = totalFrameSize;

    __sdf_exit("sdfEncode OK, total size: %d", totalFrameSize);
    return totalFrameSize; /// Return total frame size
}

static inline def sdfDecode(safeTransferContext_t* sdfContext, const void* data, int16_t size) {
    __SDF_IF_NULL(sdfContext, return STATUS_ERR;);
    __SDF_IF_NULL(data, return STATUS_ERR;);

    const uint8_t* rxData = (const uint8_t*)data;
    def result = OK; /// Default: processed, but no full packet

    for (int16_t i = 0; i < size; i++) {
        uint8_t b = rxData[i];
        
        /// Check for buffer overflow BEFORE writing
        if (sdfContext->buffBitInd >= sdfContext->buffSize) {
            __sdf_err("SDF buffer overflow!");
            sdfContext->__isReceiving = 0; /// Reset state
            result = STATUS_ERR;
            break; /// Stop processing chunk
        }

        switch (sdfContext->__isReceiving) {
            /// STATE 0: Wait for Start Frame
            case 0:
                if (b == __START_FRAME__) {
                    sdfContext->buffBitInd = 0; /// Reset buffer index
                    sdfContext->buffPtr[sdfContext->buffBitInd++] = b;
                    sdfContext->__isReceiving = 1; /// Go to next state
                }
                break;

            /// STATE 1: Wait for Header (ID/LEN)
            case 1:
                sdfContext->buffPtr[sdfContext->buffBitInd++] = b;
                sdfContext->__1st_byte = b; /// Load ID and LEN into context
                
                if (sdfContext->len > 15) { /// Max payload is 15
                    __sdf_err("Invalid frame length: %d", sdfContext->len);
                    sdfContext->__isReceiving = 0; /// Reset
                    result = STATUS_ERR;
                } else if (sdfContext->len == 0) { /// 0-byte payload
                    sdfContext->__isReceiving = 3; /// Skip payload, wait for CRC
                } else {
                    sdfContext->__continuousLogicCount = sdfContext->len; /// Store bytes remaining
                    sdfContext->__isReceiving = 2; /// Go to payload state
                }
                break;

            /// STATE 2: Wait for Payload bytes
            case 2:
                sdfContext->buffPtr[sdfContext->buffBitInd++] = b;
                if (--sdfContext->__continuousLogicCount == 0) {
                    sdfContext->__isReceiving = 3; /// Done, wait for CRC
                }
                break;

            /// STATE 3: Wait for CRC High-Byte
            case 3:
                sdfContext->buffPtr[sdfContext->buffBitInd++] = b;
                sdfContext->__isReceiving = 4;
                break;

            /// STATE 4: Wait for CRC Low-Byte
            case 4:
                sdfContext->buffPtr[sdfContext->buffBitInd++] = b;
                sdfContext->__isReceiving = 5;
                break;

            /// STATE 5: Wait for End Frame
            case 5:
                if (b == __END_FRAME__) {
                    /// Frame end received, check CRC
                    /// Data to check: [HEADER] + [PAYLOAD] + [CRC]
                    /// Location: &sdfContext->buffPtr[1]
                    /// Size: 1 (Hdr) + len (Payload) + 2 (CRC)
                    int16_t checkSize = 1 + sdfContext->len + 2;
                    
                    if (__sdfCRCCheck(&sdfContext->buffPtr[1], checkSize) == OK) {
                        /// Valid frame!
                        result = sdfContext->len; /// Return payload length
                    } else {
                        __sdf_err("SDF CRC check failed!");
                        result = STATUS_ERR;
                    }
                } else {
                    __sdf_err("Invalid end frame byte: 0x%02X", b);
                    result = STATUS_ERR;
                }
                
                /// Reset state machine regardless of success or failure
                sdfContext->__isReceiving = 0;
                break;
        } // end switch

        /// If an error occurred, stop processing this chunk
        if (result == STATUS_ERR) break;
        
        /// If a valid packet was found, stop processing and return
        if (result > 0) break;

    } // end for loop

    return result;
}

/// EXTEND FOR C++ //////////////////////////////////////////////////
#ifdef __cplusplus                                                  ///
}                                                                   ///
#endif                                                              ///
/////////////////////////////////////////////////////////////////////
#endif // __SAFE_UART_COMMUNICATE_H__