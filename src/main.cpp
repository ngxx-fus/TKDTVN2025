#include "../include/localHelperAndUtil.h"

void Task01(void* pv){
    __entry("Task01()");
    
    while(1){

        vTaskDelay(1);
    }

    __exit("Task01()");
}

void Task02(void* pv){
    

    while(1){
        vTaskDelay(1);
    }
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


    __exit("setup()");
}

void loop(){
    __log("running...");
    delay(1000);
}
