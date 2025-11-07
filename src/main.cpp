#include "main.h"

void setup(){
    /// For serial log | Baudrate: 115200
    Serial.begin(115200);
    /// For semaphore log
    semaphoreLogInit();
    /// Other set-up
    __entry("setup()");
    /// Add Task01 and Task02 and Task03
    __sys_log("[+] Add Task01");
    xTaskCreate(Task01, "Task01", 2048, NULL, 1, NULL);
    __sys_log("[+] Add Task02");
    xTaskCreate(Task02, "Task02", 2048, NULL, 1, NULL);
    #if (FIREBASE_SYNC_EN == 1)
        wfInit();
        fbInit();
        __sys_log("[+] Add Task03");
        xTaskCreate(Task03, "Task03", 2048, NULL, 1, NULL);
    #endif

    __exit("setup()");
}

void loop(){
    /// Infinity lock :>
    __sys_log("[loop] Put loop() to infinity sleep!");
    vTaskDelay(portMAX_DELAY);
}
