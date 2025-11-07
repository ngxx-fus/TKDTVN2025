#ifndef __HCRS04_WRAP_H__
#define __HCRS04_WRAP_H__

#include "stdint.h"                                     /// For uint8_t, int16_t, ...
#include "../../include/projectConfig.h"                /// For *_EN, ... macros
#include "../cReturnType/cReturnType.h"                 /// For def, STATUS_OKE, STATUS_ERR*
#include "../espSerialWrap/espSerialWrap.h"             /// For __sys_log, __entry, __exit,...

#ifndef HCSR04_SERSOR_NUM
    #define HCSR04_SERSOR_NUM   4
#endif

typedef union hcrs04Pin_t{
    struct {
        uint8_t T0, T1, T2, T3, E0, E1, E2, E3;
    };
    uint8_t arr[8];
} hcrs04EchoPin_t;

typedef union hcrs04Data_t{
    struct {
        int16_t __0         : 16;
        int16_t __1         : 16;
        int16_t __2         : 16;
        int16_t __3         : 16;
    };
    uint8_t arr[sizeof(int16_t) * 4];
}   mpu6050Data_t;

typedef union hcrs04Dev_t{
    struct {
        mpu6050Data_t   d;
        hcrs04Pin_t     pin;
    };
    uint8_t arr[sizeof(mpu6050Data_t) + sizeof(hcrs04Pin_t)];
}hcrs04Dev_t;

extern hcrs04Dev_t  hcrs04Dev;

void hcsr04Init(){
     = {
        .pin_bit_mask = 0,
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config_t pin;
    /// TRIGGER
    pin.intr_type       = GPIO_INTR_DISABLE;
    pin.pull_up_en      = GPIO_PULLUP_DISABLE;
    pin.pull_down_en    = GPIO_PULLDOWN_DISABLE;
    pin.mode            = GPIO_MODE_OUTPUT;
    for(int i = 0; i < 4; ++i) pin.pin_bit_mask |= __mask64(hcrs04Dev.pin.arr[i]); 
    gpio_config(&pin);
    /// ECHO
    pin.pin_bit_mask    = 0;
    for(int i = 4; i < 8; ++i) pin.pin_bit_mask |= __mask64(hcrs04Dev.pin.arr[i]); 
    pin.intr_type       = GPIO_INTR_DISABLE;
    pin.pull_up_en      = GPIO_PULLUP_DISABLE;
    pin.pull_down_en    = GPIO_PULLDOWN_DISABLE;
    pin.mode            = GPIO_MODE_INPUT;
    gpio_config(&pin);
}

def hcsr04Measure(int i){
    if( i >= HCSR04_SERSOR_NUM || i < 0) return STATUS_ERR;
    /// Trigger 
    GPIO.out_w1ts = __mask32(hcrs04Dev.pin.arr[i]);
    esp_rom_delay_us(10);
    GPIO.out_w1tc = __mask32(hcrs04Dev.pin.arr[i]);


    return OKE;
}





#endif