/* ===================================================
 * FILE: project_config.h (ĐÃ SỬA)
 * ===================================================*/
#ifndef __PROJECT_CONFIG_H__
#define __PROJECT_CONFIG_H__

#include "stdint.h"

#define SYS_LOG_L0_EN 1
#define SYS_LOG_L1_EN 0
#define SYS_LOG_L2_EN 0
#define SYS_LOG_err
#define SYS_LOG_L0_ENTRY 1
#define SYS_LOG_L0_EXIT 1
#define SYS_LOG_L1_ENTRY 0
#define SYS_LOG_L1_EXIT 0
#define SYS_LOG_L2_ENTRY 0
#define SYS_LOG_L2_EXIT 0

#define FIREBASE_SYNC_EN 0
#define LIGHT_TICK_EN 0
#define SENSOR_HCSR04_EN 0
#define SENSOR_MPU6050_EN 0
#define SENSOR_ATGM336H_EN 0

#if (FIREBASE_SYNC_EN == 1)
    #define FBRTDB_URL "https://tkdtvn2025-default-rtdb.asia-southeast1.firebasedatabase.app/"
    #define FB_API_KEY "AIzaSyBC-o4n9PbYDeWlKkxo-Jogn7EhwA96MZ8"
    #define FB_USER_EMAIL "nthanhphu.k22.hcmute@gmail.com"
    #define FB_USER_PASSWORD "@asjd2361qeuFASl:kHJDASsn#wer"
    #define WIFI_SSID "Wokwi-GUEST"
    #define WIFI_PASSWORD ""

    #define FBRTDB_ROOT_PATH "/"
    #define FBRTDB_MPU6050_PATH FBRTDB_ROOT_PATH "mpu6050/"
#endif

#if (LIGHT_TICK_EN == 1)
    #define LIGHT_TICK_PIN 2
    #define LIGHT_TICK_TIME_ON 200
    #define LIGHT_TICK_TIME_OFF 500
#endif

#if (SENSOR_HCSR04_EN == 1)
    #define HCSR04_0_PIN 19 /// Front
    #define HCSR04_1_PIN 5 /// Right
    #define HCSR04_2_PIN 18 /// Back
    #define HCSR04_3_PIN 26 /// Left (ĐÃ THAY ĐỔI TỪ 17)
#endif


#endif