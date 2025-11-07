#include "HCSR04Wrap.h"

/// #pragma message("[compile] HCSR04Wrap.c")

hcsr04Dev_t      hcsr04Dev;
hcrs04Data_t     hcsr04Data;

void hcsr04Init(){
    gpio_config_t pin;
    /// Config TRIGGER pin
    pin.intr_type       = GPIO_INTR_DISABLE;
    pin.pull_up_en      = GPIO_PULLUP_DISABLE;
    pin.pull_down_en    = GPIO_PULLDOWN_DISABLE;
    pin.mode            = GPIO_MODE_OUTPUT;
    for(int i = 0; i < HCSR04_SERSOR_NUM; ++i) {
        pin.pin_bit_mask |= __mask64(hcsr04Dev.arr[i]);
    } 
    gpio_config(&pin);
    /// Config ECHO pin
    pin.pin_bit_mask    = 0;
    for(int i = HCSR04_SERSOR_NUM; i < 2 * HCSR04_SERSOR_NUM; ++i) {
        pin.pin_bit_mask |= __mask64(hcsr04Dev.arr[i]); 
    }
    pin.intr_type       = GPIO_INTR_DISABLE;
    pin.pull_up_en      = GPIO_PULLUP_DISABLE;
    pin.pull_down_en    = GPIO_PULLDOWN_DISABLE;
    pin.mode            = GPIO_MODE_INPUT;
    gpio_config(&pin);
}

def hcsr04Config(hcsr04Dev_t * dev){
    for(int i = 0; i < HCSR04_SERSOR_NUM * 2; ++i){
        hcsr04Dev.arr[i] = dev->arr[i];
    }
}

def hcsr04Measure(int i){
    if( i >= HCSR04_SERSOR_NUM || i < 0) return STATUS_ERR;
    uint64_t GPIO_INPUT = 0;
    uint32_t ECHO_PIN_MASK = 0, TRIGGER_PIN_MASK = 0;
    TRIGGER_PIN_MASK    = __mask32(hcsr04Dev.arr[i]);
    ECHO_PIN_MASK       = __mask32(hcsr04Dev.arr[i+HCSR04_SERSOR_NUM]);
    /// Trigger 
    GPIO.out_w1ts = TRIGGER_PIN_MASK;
    esp_rom_delay_us(10);
    GPIO.out_w1tc = TRIGGER_PIN_MASK;
    /// Wait for Echo HIGH
    int64_t timeHigh = esp_timer_get_time();
    while(1){
        GPIO_INPUT = GPIO.in;
        if((ECHO_PIN_MASK & GPIO_INPUT) != 0) {
            timeHigh = esp_timer_get_time();
            break;
        }
        if(esp_timer_get_time() - timeHigh > 25000) {
            /// Error occured!
            return STATUS_ERR;
        }
    }
    /// Wait for Echo LOW
    int64_t timeLow = esp_timer_get_time();
    while(1){
        GPIO_INPUT = GPIO.in;
        if((ECHO_PIN_MASK & GPIO_INPUT) == 0) {
            timeLow = esp_timer_get_time();
            break;
        }
        if(esp_timer_get_time() - timeLow > 30000) {
            /// Error occured!
            return STATUS_ERR;
        }
    }
    hcsr04Data.arr[i] = ((timeLow - timeHigh) *  17) / (1000);
    return STATUS_OKE;
}

def hcsr04MeasureAll(){
    uint64_t GPIO_INPUT = 0;
    uint32_t ECHO_PIN_MASK = 0, TRIGGER_PIN_MASK = 0;
    /// Measure all sensors
    for(int i = 0; i < HCSR04_SERSOR_NUM; ++i){
        TRIGGER_PIN_MASK    = __mask32(hcsr04Dev.arr[i]);
        ECHO_PIN_MASK       = __mask32(hcsr04Dev.arr[i+HCSR04_SERSOR_NUM]);
        /// Trigger 
        GPIO.out_w1ts = TRIGGER_PIN_MASK;
        esp_rom_delay_us(10);
        GPIO.out_w1tc = TRIGGER_PIN_MASK;
        /// Wait for Echo HIGH
        int64_t timeHigh = esp_timer_get_time();
        while(1){
            GPIO_INPUT = GPIO.in;
            if((ECHO_PIN_MASK & GPIO_INPUT) != 0) {
                timeHigh = esp_timer_get_time();
                break;
            }
            if(esp_timer_get_time() - timeHigh > 25000) {
                __sys_err("[hcsr04MeasureAll] Loop-%d: WaitingForHigh timeout!", i);
                /// Error occured!
                return STATUS_ERR;
            }
        }
        /// Wait for Echo LOW
        int64_t timeLow = esp_timer_get_time();
        while(1){
            GPIO_INPUT = GPIO.in;
            if((ECHO_PIN_MASK & GPIO_INPUT) == 0) {
                timeLow = esp_timer_get_time();
                break;
            }
            if((timeLow = esp_timer_get_time()) - timeHigh > 300000) {
                /// Error occured!
                __sys_err("[hcsr04MeasureAll] Loop-%d: WaitingForLow timeout(%lld)!", i, (timeLow - timeHigh));
                return STATUS_ERR;
            }
        }
        hcsr04Data.arr[i] = ((timeLow - timeHigh) *  17) / (1000);
        /// Wait 60ms before start the next measure process
        esp_rom_delay_us(60);
        vTaskDelay(0);
    }

    return STATUS_OKE;
}
