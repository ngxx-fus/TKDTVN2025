#include "main.h"

void yourTask(void* pv){
    __entry("yourTask()");
    /// your loop code
    /// 


    while(1){
        /// your loop
        /// your loop get data from sensor
        /// log to serial
        /// i recommend to use GLOBAL variable to store them.
        /// Check MPU6050 for the structure.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    __exit("yourTask()");
}

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
        xTaskCreate(Task04, "Task03", 2048, NULL, 1, NULL);
    #endif
    /// Add your stat-up/init script for your sensor ATGM336H
    /// ....
    /// Add in mainTask, if stat-up allocate more than 2048B memory
    /// it will cause overflow
    /// Start-up/Init means you send config to your Sensor
    /// Or initialize HW for communication (UART)
    /// In summary, you will init twice times, one for Hardware, one for sensor
    __sys_log("[+] Add yourTask");
    xTaskCreate(yourTask, "yourTask", 2048, NULL, 1, NULL);
    __exit("setup()");
}

void loop(){
    /// Infinity lock :>
    __sys_log("[loop] Put loop() to infinity sleep!");
    vTaskDelay(portMAX_DELAY);
}
