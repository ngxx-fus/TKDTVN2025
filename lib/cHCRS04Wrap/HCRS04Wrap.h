#ifndef __HCRS04_WRAP_H__
#define __HCRS04_WRAP_H__

#include "stdint.h"                                     /// For uint8_t, int16_t, ...
#include "../../include/projectConfig.h"                /// For *_EN, ... macros
#include "../cReturnType/cReturnType.h"                 /// For def, STATUS_OKE, STATUS_ERR*
#include "../espSerialWrap/espSerialWrap.h"             /// For __sys_log, __entry, __exit,...

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

}

void hcsr04Measure(){
    
}





#endif