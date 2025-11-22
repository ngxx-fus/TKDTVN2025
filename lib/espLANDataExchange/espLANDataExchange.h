#pragma once

#include "../../include/projectConfig.h"

#if (LAN_DATA_EXCHANGE_EN == 1)

/// #pragma message("[include] espLANDataExchange.h")

#include <Arduino.h>

#if (defined(ESP32) && (ESP_USE_CRC_HW == 1))
    #include <esp_rom_crc.h>
#endif

#include <WiFi.h>
#include <WiFiUdp.h>

#include "../espSerialWrap/espSerialWrap.h"
#include "../espSoftTimer/espSoftTimer.h"
#include "../cReturnType/cReturnType.h"

/* --- Protocol Definitions --- */
// #define ELDE_RX_BUF_SIZE            512

/// Frame Markers
// #define ELDE_FRAME_BEGIN            "FRAME_BEGIN"
// #define ELDE_FRAME_END              "FRAME_END"
// #define ELDE_TAG_CRC                "CRC"

/// IDs Mapping
// #define ID_SENSOR_BASE          40
// #define ID_SENSOR_MPU6050       (ID_SENSOR_BASE + 1)
// #define ID_SENSOR_HCSR04        (ID_SENSOR_BASE + 2)
// #define ID_SENSOR_ATGM336H      (ID_SENSOR_BASE + 3)

// #define ID_STATUS_BASE          200
// #define ID_STATUS_OKE           (ID_STATUS_BASE)
// #define ID_STATUS_ERR           (ID_STATUS_BASE + 1)

/// CRC Polynomial Default
#ifndef CRC_POLYNOMIAL
    #define CRC_POLYNOMIAL      0x1021
#endif

/// Hardware CRC Selection Default
#ifndef ESP_USE_CRC_HW
    #define ESP_USE_CRC_HW      0
#endif

/// Structure defining a LAN Host
typedef struct {
    const char* ip;         ///< IP Address string
    const char* hostName;   ///< Readable name
    struct {
        uint16_t udp;       ///< UDP Port
        uint16_t tcp;       ///< TCP Port
    } port;
} espLANHost_t;

/// Object representing THIS device
extern espLANHost_t thisESP;
/// Object representing PC Host
extern espLANHost_t broadcastHost;

/* --- Function Prototypes --- */

/// Initialize Wi-Fi and UDP
def eldeInit();

/// Update self IP address
void eldeUpdateSelfIP();

/// @brief Calculate CRC16 with HW/SW support based on ESP_USE_CRC_HW
uint16_t computeCRC16(void* ByteArr, int ByteArrSize, int16_t CRCInitValue);

/// @brief Send frame: <FRAME_BEGIN><ID><DATA...><CRC Tag><CRCHigh><CRCLow><FRAME_END>
def eldeSendFrame(uint8_t id, void* data, uint8_t len, const espLANHost_t* target);

/// @brief Poll for incoming UDP packets
void eldePoll();

/// @brief Check UDP buffer, skip loopbacks, read first valid packet within timeout
/// @param timeoutMs Maximum time (in ms) to spend processing the buffer to avoid blocking
/// @return 1 if valid packet found, 0 on timeout or empty buffer
def hasReceivedFrame(uint32_t timeoutMs);

/// @brief Validate frame, Check CRC, Parse ID and Copy Payload
/// @param outId Pointer to store parsed ID
/// @param outDataLen Pointer to store actual payload length
/// @param buffer Destination buffer to copy data
/// @param bufferSize Size of destination buffer
def getFrameData(uint8_t *outId, int *outDataLen, void* buffer, int bufferSize);

#endif // (LAN_DATA_EXCHANGE_EN == 1)