#include "../../include/projectConfig.h"

#if (SENSOR_ATGM336H_EN == 1)
#ifndef __ATGM336H_WRAP_H__
#define __ATGM336H_WRAP_H__

#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <stdint.h>
#include <atomic> // Required for std::atomic

/* --- Configuration Macros --- */
#if 1 /// Default pin configuration
    #ifndef SENSOR_GPS_EN
        #define SENSOR_GPS_EN 1
    #endif
    #ifndef GPS_RX_PIN
        #define GPS_RX_PIN 33
    #endif
    #ifndef GPS_TX_PIN
        #define GPS_TX_PIN 32
    #endif
    #ifndef GPS_BAUD
        #define GPS_BAUD 115200
    #endif
    #ifndef GPS_PPS_EN
        #define GPS_PPS_EN 0
    #endif
    #ifndef GPS_PPS_PIN
        #define GPS_PPS_PIN 35
    #endif
    #ifndef GPS_SERIAL_NUM
        #define GPS_SERIAL_NUM 2
    #endif
#endif

/* --- Flag Bit Definitions --- */
#define GPS_UPDATA_DATE_TIME (1 << 0)
#define GPS_UPDATA_LOCATION  (1 << 1)
#define GPS_UPDATA_OTHERS    (1 << 2)

/* --- Data Types --- */
typedef union atgm336hData_t{
    struct __attribute__((packed)) {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
        uint8_t day;
        uint8_t month;
        uint16_t year;

        float latitude;
        float longitude;

        float speed_kmh;
        uint8_t sats;
    };
    uint8_t arr[20]; 
} atgm336hData_t;

/* --- Global Externs --- */
extern HardwareSerial       gpsSerial;
extern TinyGPSPlus          gpsDevice;
extern atgm336hData_t       gpsData;
extern std::atomic<uint8_t> gpsDataUpdateFlag; // Use std::atomic for thread safety

/* --- Flag Manipulation Macros (Atomic) --- */
/// @brief Set a specific flag bit atomically
#define gpsSetUpdateFlag(flag)   (gpsDataUpdateFlag.fetch_or(flag, std::memory_order_relaxed))

/// @brief Clear a specific flag bit atomically
#define gpsClearUpdateFlag(flag) (gpsDataUpdateFlag.fetch_and(~(flag), std::memory_order_relaxed))

/// @brief Check if a specific flag bit is set
#define gpsGetUpdateFlag(flag)   (gpsDataUpdateFlag.load(std::memory_order_relaxed) & (flag))

#endif /// __ATGM336H_WRAP_H__
#endif /// (SENSOR_ATGM336H_EN == 1)