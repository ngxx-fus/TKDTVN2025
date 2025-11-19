#pragma once

#include "../../include/projectConfig.h"

#if (LAN_DATA_EXCHANGE_EN == 1)

/// #pragma message("[include] espLANDataExchange.h")

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "../espSerialWrap/espSerialWrap.h"
#include "../espSoftTimer/espSoftTimer.h"
#include "../cReturnType/cReturnType.h"

/// Enumeration for internal flags and states
typedef enum {
    ELDE_FLG_NONE           = 0x00, /// No flags set
    ELDE_FLG_UDP_READY      = 0x01, /// UDP Socket is initialized
    ELDE_FLG_TCP_CONNECTED  = 0x02, /// TCP Client is connected
    ELDE_FLG_RX_PENDING     = 0x04  /// Data received and waiting in buffer
} eldeFlag_t;

/// Structure defining a LAN Host (Target or Self)
typedef struct {
    const char* ip;         ///< IP Address string (e.g., "192.168.1.100")
    const char* hostName;   ///< Readable name for debug
    struct {
        uint16_t udp;       ///< UDP Port for listening/sending
        uint16_t tcp;       ///< TCP Port for listening/sending
    } port;
} espLANHost_t;

/// Object representing THIS device (Self)
extern espLANHost_t thisESP;

/// Initialize Wi-Fi with Dynamic Backoff Strategy (Non-blocking)
void wfInit();

/// Update thisESP.ip with current WiFi local IP (Call after WiFi connected)
void eldeUpdateSelfIP();

/// Initialize LAN Data Exchange (Start UDP listening on thisESP.port.udp)
def eldeInit();

/// Send single byte via UDP to specific target
/// @param target Pointer to destination host structure
/// @param data Byte to send
def espUDPSendByte(const espLANHost_t* target, uint8_t data);

/// Send byte array via UDP to specific target
/// @param target Pointer to destination host structure
/// @param data Pointer to data array
/// @param len Length of data
def espUDPSendByteArr(const espLANHost_t* target, const uint8_t* data, size_t len);

/// Send single byte via TCP to specific target
/// @param target Pointer to destination host structure
/// @param data Byte to send
def espTCPSendByte(const espLANHost_t* target, uint8_t data);

/// Send byte array via TCP to specific target
/// @param target Pointer to destination host structure
/// @param data Pointer to data array
/// @param len Length of data
def espTCPSendByteArr(const espLANHost_t* target, const uint8_t* data, size_t len);

/// Poll for incoming UDP packets (Call in loop)
void eldePoll();

/// Get received UDP byte
/// @param outData Pointer to store received byte
def espUDPReceiveByte(uint8_t* outData);

/// Get received UDP buffer
/// @param buffer Pointer to destination buffer
/// @param maxLen Maximum bytes to read
def espUDPReceiveByteArr(uint8_t* buffer, size_t maxLen);

/// Get received TCP byte
/// @param outData Pointer to store received byte
def espTCPReceiveByte(uint8_t* outData);

/// Get received TCP buffer
/// @param buffer Pointer to destination buffer
/// @param maxLen Maximum bytes to read
def espTCPReceiveByteArr(uint8_t* buffer, size_t maxLen);

#endif // (LAN_DATA_EXCHANGE_EN == 1)