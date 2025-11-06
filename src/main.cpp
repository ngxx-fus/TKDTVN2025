#include "main.h"

void Task01(void* pv){
    __entry("Task01()");
    /// Task01 : Blink LED with period 200 tick
    while(1){
        GPIO.out_w1tc = __mask32(HM_LED0_PIN);
        vTaskDelay(100);
        GPIO.out_w1ts = __mask32(HM_LED0_PIN);
        vTaskDelay(100);
    }

    __exit("Task01()");
}

void Task02(void* pv){
    __entry("Task02()");
    /// Task02 : Blink LED with period 700 tick
    while(1){
        GPIO.out_w1tc = __mask32(HM_LED1_PIN);
        vTaskDelay(350);
        GPIO.out_w1ts = __mask32(HM_LED1_PIN);
        vTaskDelay(350);
    }
    __exit("Task02()");
}

void Task03(void *pv){
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
        vTaskDelay(pdMS_TO_TICKS(350));
    }

    __exit("Task03()");
    vTaskDelete(NULL);
}

void Task04(void* pv){
    __entry("Task04()");
    while(1){
        if(fbUploadMPU6050Data()!=STATUS_OKE) __sys_log("[Task04] Upload failed!");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    __exit("Task04()");
}

void setup(){
    /// For serial log | Baudrate: 115200
    Serial.begin(115200);
    /// For semaphore log
    semaphoreLogInit();
    /// Other set-up
    __entry("setup()");

    /// Set-up LED for Task01 and Task02
    gpio_config_t outPin = {
        .pin_bit_mask = __masks64(HM_LED0_PIN, HM_LED1_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&outPin);

    wfInit();
    fbInit();

    /// Add Task01 and Task02 and Task03
    __sys_log("[+] Add Task01");
    xTaskCreate(Task01, "Task01", 2048, NULL, 1, NULL);
    __sys_log("[+] Add Task02");
    xTaskCreate(Task02, "Task02", 2048, NULL, 1, NULL);
    __sys_log("[+] Add Task03");
    xTaskCreate(Task03, "Task03", 2048, NULL, 1, NULL);
    __sys_log("[+] Add Task04");
    xTaskCreate(Task04, "Task04", 2048, NULL, 1, NULL);

    __exit("setup()");
}

void loop(){
    /// Infinity lock :>
    __sys_log("[loop] Put loop() to infinity sleep!");
    vTaskDelay(portMAX_DELAY);
}
