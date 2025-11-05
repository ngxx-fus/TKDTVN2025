#include "../include/localHelperAndUtil.h"

void Task01(void* pv){
    __entry("Task01()");
    
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
    

    while(1){
        GPIO.out_w1tc = __mask32(HM_LED1_PIN);
        vTaskDelay(300);
        GPIO.out_w1ts = __mask32(HM_LED1_PIN);
        vTaskDelay(300);
    }
    __exit("Task02()");
}

void setup(){
    __entry("setup()");
    Serial.begin(115200);

    gpio_config_t outPin = {
        .pin_bit_mask = __masks64(HM_LED0_PIN, HM_LED1_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&outPin);

    __log("[+] Add Task01");
    xTaskCreate(Task01, "Task01", 2048, NULL, 1, NULL);
    __log("[+] Add Task01");
    xTaskCreate(Task02, "Task02", 2048, NULL, 1, NULL);


    __exit("setup()");
}

void loop(){
    __log("running...");
    delay(1000);
}
