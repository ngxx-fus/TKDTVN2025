#pragma once
#include <Arduino.h>

// ===== GPS config =====
#ifndef SENSOR_GPS_EN
#define SENSOR_GPS_EN 1
#endif
#ifndef GPS_RX_PIN
#define GPS_RX_PIN 16   // ESP32 RX2  <- GPS TX
#endif
#ifndef GPS_TX_PIN
#define GPS_TX_PIN 17   // ESP32 TX2  -> GPS RX (không bắt buộc)
#endif
#ifndef GPS_BAUD
#define GPS_BAUD 9600
#endif
#ifndef GPS_USE_PPS
#define GPS_USE_PPS 0
#endif
#ifndef GPS_PPS_PIN
#define GPS_PPS_PIN 34
#endif

#ifdef __cplusplus
extern "C" {
#endif
void gpsInit();
void TaskGPS(void* pv);
#ifdef __cplusplus
}
#endif
