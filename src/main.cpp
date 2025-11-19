#include "main.h"

void setup(){
    /// For serial log | Baudrate: 115200
    Serial.begin(115200);
    /// For semaphore log
    semaphoreLogInit();
    /// Other set-up
    __entry("setup()");

    #if (LIGHT_TICK_EN == 1)
        /// Add TaskLightTick
        __sys_log("[setup] [+] Add TaskLightTick");
        xTaskCreatePinnedToCore(TaskLightTick, "TaskLightTick", 2048, NULL, 1, NULL, 1);
    #endif
    
    #if (SENSOR_MPU6050_EN == 1)
        __sys_log("[setup] [+] Add TaskMPU6050");
        xTaskCreate(TaskMPU6050, "TaskMPU6050", 2048, NULL, 1, NULL);
    #endif

    #if (SENSOR_HCSR04_EN == 1)
        __sys_log("[setup] [+] Add TaskHCSR04");
        xTaskCreate(TaskHCSR04, "TaskHCSR04", 2048, NULL, 1, NULL);
    #endif

    #if (SENSOR_ATGM336H_EN == 1)
        __sys_log("[setup] [+] Add TaskATGM336H");
        xTaskCreate(TaskATGM336H, "TaskATGM336H", 2048, NULL, 1, NULL);
    #endif

    #if (LAN_DATA_EXCHANGE_EN == 1) || (FIREBASE_SYNC_EN == 1)
        wfInit();
    #endif

    #if (LAN_DATA_EXCHANGE_EN == 1)
        __sys_log("[setup] [+] Add TaskLANDataExchange");
        xTaskCreatePinnedToCore(TaskLANDataExchange, "TaskLANDataExchange", 8192, NULL, 1, NULL, 0);
    #endif

    #if (FIREBASE_SYNC_EN == 1)
        fbInit();
        __sys_log("[setup] [+] Add TaskFirebaseSync");
        xTaskCreatePinnedToCore(TaskFirebaseSync, "TaskFirebaseSync", 8192, NULL, 1, NULL, 1);
    #endif

    __exit("setup()");
}

void loop(){
    __sys_log("[loop] Put loop() to infinity sleep!");
    vTaskDelay(portMAX_DELAY);
}
