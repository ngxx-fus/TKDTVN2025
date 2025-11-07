#pragma once
#include <Arduino.h>
#include <cReturnType.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>   // API mới của Mobizt
#include "../MPU6050Wrap/MPU6050Wrap.h"   // để lấy mpuData
#include <LittleFs.h>

// ====== Các macro cấu hình bắt buộc ======
#ifndef FBRTDB_URL
#  error "You must define FBRTDB_URL (e.g. https://xxx-default-rtdb.asia-southeast1.firebasedatabase.app)"
#endif
#ifndef FB_API_KEY
#  error "You must define FB_API_KEY"
#endif
#ifndef FB_USER_EMAIL
#  error "You must define FB_USER_EMAIL"
#endif
#ifndef FB_USER_PASSWORD
#  error "You must define FB_USER_PASSWORD"
#endif
#ifndef WIFI_SSID
#  error "You must define WIFI_SSID"
#endif
#ifndef WIFI_PASSWORD
#  error "You must define WIFI_PASSWORD"
#endif

#ifndef FBRTDB_ROOT_PATH
#  define FBRTDB_ROOT_PATH "/"
#endif
#ifndef FBRTDB_MPU6050_PATH
#  define FBRTDB_MPU6050_PATH FBRTDB_ROOT_PATH "mpu6050"
#endif

// ====== Đối tượng Firebase (API mới) ======
extern FirebaseData fbdo;       // thay cho firebaseData kiểu cũ
extern FirebaseAuth auth;
extern FirebaseConfig fbconfig;

// ====== API ======
DEFAULT_RETURN_STATUS wfIsConnected();
void wfInit();
void fbInit();
DEFAULT_RETURN_STATUS fbUploadMPU6050Data();
