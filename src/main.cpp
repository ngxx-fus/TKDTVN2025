#include "main.h"

void setup(){
    /// For serial log | Baudrate: 115200
    Serial.begin(115200);
    /// For semaphore log
    semaphoreLogInit();
    /// Other set-up
    __entry("setup()");

    #if (LIGHT_TICK_EN == 1)
        /// Add TaskLightTick and TaskMPU6050 and TaskFirebaseSync
        __sys_log("[setup] [+] Add TaskLightTick");
        xTaskCreate(TaskLightTick, "TaskLightTick", 2048, NULL, 1, NULL);
    #endif
    
    #if (SENSOR_MPU6050_EN == 1)
        __sys_log("[setup] [+] Add TaskMPU6050");
        xTaskCreate(TaskMPU6050, "TaskMPU6050", 2048, NULL, 1, NULL);
    #endif

    #if (SENSOR_HCSR04_EN == 1)
        __sys_log("[setup] [+] Add TaskHCSR04");
        xTaskCreate(TaskHCSR04, "TaskHCSR04", 2048, NULL, 1, NULL);
    #endif

    #if (FIREBASE_SYNC_EN == 1)
        wfInit();
        fbInit();
        __sys_log("[setup] [+] Add TaskFirebaseSync");
        xTaskCreate(TaskFirebaseSync, "TaskFirebaseSync", 8192, NULL, 1, NULL);
    #endif

    __exit("setup()");
}

void loop(){
    /// Infinity lock :>
    __sys_log("[loop] Put loop() to infinity sleep!");
    vTaskDelay(portMAX_DELAY);
}
