/* ===================================================
 * FILE: include/main.h (PHIÊN BẢN SẠCH)
 * ===================================================*/

#pragma once 

#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// +++ ĐỊNH NGHĨA CHÂN PIN CÒN THIẾU +++
// (Thay số 25 bằng chân GPIO bạn dùng cho Trigger)
#define HCSR04_C_PIN 25 

// --- Bao gồm các file thư viện (đã sửa đường dẫn) ---
#include "localHelperAndUtil.h" 
#include "extLibraryAndDriver.h"

// --- Khai báo Semaphore ---
extern SemaphoreHandle_t logMutex; 

// --- Khai báo các Task (Declarations) ---
// (KHÔNG có code, chỉ có dấu chấm phẩy)
#if (LIGHT_TICK_EN == 1)
    void TaskLightTick(void* pv);
#endif

#if (SENSOR_MPU6050_EN == 1)
    void TaskMPU6050(void *pv);
#endif

#if (FIREBASE_SYNC_EN == 1)
    void TaskFirebaseSync(void* pv);
#endif

#if (SENSOR_HCSR04_EN == 1)
    // Cần khai báo 'DEFAULT_RETURN_STATUS' trước khi dùng
    #include <cReturnType.h> 
    void TaskHCSR04(void *pv);
#endif

// --- Include các file .h của task khác ---
#include "gps_task.h"