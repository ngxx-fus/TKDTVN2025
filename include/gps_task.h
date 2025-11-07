/* ===================================================
 * FILE: src/gps_task.h
 * (PHIÊN BẢN SẠCH - KHÔNG CÓ Task01, 02)
 * ===================================================*/

#pragma once
#include <Arduino.h>

// ===== GPS config (tùy dây của bạn) =====
#ifndef SENSOR_GPS_EN
#define SENSOR_GPS_EN 1
#endif

#ifndef GPS_RX_PIN
#define GPS_RX_PIN 16   // ESP32 RX2  <- GPS TX
#endif

#ifndef GPS_TX_PIN
#define GPS_TX_PIN 17   // ESP32 TX2  -> GPS RX
#endif

#ifndef GPS_BAUD
#define GPS_BAUD 9600
#endif

#ifndef GPS_USE_PPS
#define GPS_USE_PPS 0   // 1 nếu bạn thật sự có nối PPS
#endif

#ifndef GPS_PPS_PIN
#define GPS_PPS_PIN 34  // G34/G35 (input-only)
#endif

// ===== API (Khai báo hàm) =====
void gpsInit();
void TaskGPS(void* pv);