#include "Arduino.h"

#include "localHelperAndUtil.h"
#include "extLibraryAndDriver.h"

#if (LIGHT_TICK_EN == 1)
    void Task01(void* pv){
        __entry("Task01()");
        /// Set-up LED for Task01 and Task02
        gpio_config_t outPin = {
            .pin_bit_mask = __masks64(LIGHT_TICK_PIN),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&outPin);
        /// Task01 : Blink LED with period 200 tick
        while(1){
            GPIO.out_w1tc = __mask32(LIGHT_TICK_PIN);
            vTaskDelay(LIGHT_TICK_TIME_ON);
            GPIO.out_w1ts = __mask32(LIGHT_TICK_PIN);
            vTaskDelay(LIGHT_TICK_TIME_OFF);
        }
        __exit("Task01()");
    }
#endif

#if (SENSOR_MPU6050_EN == 1)
    void Task02(void *pv){
        __entry("Task03()");

        // Init MPU6050
        if (mpu6050Init() != STATUS_OKE) {
            __sys_log("[Task03] "  "MPU6050 connection failed!");
            vTaskDelete(NULL);
        } else {
            __sys_log("[Task03] "  "MPU6050 connected successfully.");
        }
        // Main loop
        while (1) {
            mpu6050Measure();
            __sys_log("[Task03] a[x: %d, y: %d, z: %d] g[x: %d, y: %d, z: %d]", 
                    mpuData.ax, mpuData.ay, mpuData.az, mpuData.gx, mpuData.gy, mpuData.gz);
            vTaskDelay(pdMS_TO_TICKS(350));
        }

        __exit("Task03()");
        vTaskDelete(NULL);
    }
#endif

#if (FIREBASE_SYNC_EN == 1)
    void Task03(void* pv){
        __entry("Task04()");
        while(1){
            if(fbUploadMPU6050Data()!=STATUS_OKE) __sys_log("[Task04] Upload failed!");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        __exit("Task04()");
    }
#endif

