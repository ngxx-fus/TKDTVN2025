/// #pragma message("[include] main.h")

#include "Arduino.h"

#include "localHelperAndUtil.h"
#include "extLibraryAndDriver.h"

#if (LIGHT_TICK_EN == 1)
    void TaskLightTick(void* pv){
        __entry("TaskLightTick()");
        /// Set-up LED for TaskLightTick and TaskMPU6050
        gpio_config_t outPin = {
            .pin_bit_mask = __masks64(LIGHT_TICK_PIN),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&outPin);
        /// TaskLightTick : Blink LED with period 200 tick
        __sys_log("[TaskLightTick] Info: GPIO=%d, TimeOn=%d TimeOff=%d", 
                    LIGHT_TICK_PIN, LIGHT_TICK_TIME_ON, LIGHT_TICK_TIME_OFF);
        while(1){
            GPIO.out_w1tc = __mask32(LIGHT_TICK_PIN);
            vTaskDelay(LIGHT_TICK_TIME_ON);
            GPIO.out_w1ts = __mask32(LIGHT_TICK_PIN);
            vTaskDelay(LIGHT_TICK_TIME_OFF);
        }
        __exit("TaskLightTick()");
    }
#endif

#if (SENSOR_MPU6050_EN == 1)
    void TaskMPU6050(void *pv){
        __entry("TaskMPU6050()");

        // Init MPU6050
        if (mpu6050Init() != STATUS_OKE) {
            __sys_log("[TaskMPU6050] "  "MPU6050 connection failed!");
            vTaskDelete(NULL);
        } else {
            __sys_log("[TaskMPU6050] "  "MPU6050 connected successfully.");
        }
        // Main loop
        while (1) {
            mpu6050Measure();
            __sys_log("[TaskMPU6050] a[x: %d, y: %d, z: %d] g[x: %d, y: %d, z: %d]", 
                    mpuData.ax, mpuData.ay, mpuData.az, mpuData.gx, mpuData.gy, mpuData.gz);
            vTaskDelay(pdMS_TO_TICKS(350));
        }

        __exit("TaskMPU6050()");
        vTaskDelete(NULL);
    }
#endif

#if (FIREBASE_SYNC_EN == 1)
    void TaskFirebaseSync(void* pv){
        __entry("TaskFirebaseSync()");
        while(1){
            if(fbUploadMPU6050Data()!=STATUS_OKE) __sys_log("[Task04] Upload failed!");
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
        __exit("TaskFirebaseSync()");
    }
#endif

#if (SENSOR_HCSR04_EN == 1)
    void TaskHCSR04(void *pv){
        def returnValue = 0;
        hcsr04Dev.T0 = hcsr04Dev.T1 = hcsr04Dev.T2 = hcsr04Dev.T3 = HCSR04_C_PIN;
        hcsr04Dev.E0 = HCSR04_0_PIN;    hcsr04Dev.E2 = HCSR04_2_PIN;
        hcsr04Dev.E1 = HCSR04_1_PIN;    hcsr04Dev.E3 = HCSR04_3_PIN;
        hcsr04Init();
        while(1){
            returnValue = hcsr04MeasureAll();
            if(returnValue != STATUS_OKE) 
            __sys_err("[TaskHCSR04] hcsr04MeasureAll(): %s", DEFAULT_RETURN_STATUS_STR(returnValue));
            __sys_log("[TaskHCSR04] F:%d R:%d B:%d L:%d", 
                        hcsr04Data.arr[0], hcsr04Data.arr[1],
                        hcsr04Data.arr[2], hcsr04Data.arr[3]);
            vTaskDelay(pdMS_TO_TICKS(250));
        }
    }
#endif

