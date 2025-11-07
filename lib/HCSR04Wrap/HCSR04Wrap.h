#ifndef __HCRS04_WRAP_H__
#define __HCRS04_WRAP_H__

/// #pragma message("[include] HCSR04Wrap.h")

#include "stdint.h"                                     /// For uint8_t, int16_t, ...
#include "../../include/projectConfig.h"                /// For *_EN, ... macros
#include "../cReturnType/cReturnType.h"                 /// For def, STATUS_OKE, STATUS_ERR*
#include "../espSerialWrap/espSerialWrap.h"             /// For __sys_log, __entry, __exit,...
#include "../cBitWiseMacro/cBitWiseMacro.h"             /// For __mask* 
#include "../cArithmeticMacro/cArithmeticMacro.h"       /// For __isnot_zero* 
#include "../freeRTOSWrap/freeRTOSWrap.h"               /// For vTaskDelay...

#ifndef HCSR04_SERSOR_NUM
    #define HCSR04_SERSOR_NUM   4
#endif

typedef union hcsr04Dev_t{
    struct {
        uint8_t T0, T1, T2, T3, E0, E1, E2, E3;
    };
    uint8_t arr[HCSR04_SERSOR_NUM * 2];
} hcsr04Dev_t;

typedef union hcrs04Data_t{
    struct {
        int16_t __0         : 16;
        int16_t __1         : 16;
        int16_t __2         : 16;
        int16_t __3         : 16;
    };
    int16_t arr[HCSR04_SERSOR_NUM];
}   hcrs04Data_t;

extern hcsr04Dev_t      hcsr04Dev;
extern hcrs04Data_t     hcsr04Data;

void hcsr04Init();
def hcsr04Measure(int i);
def hcsr04MeasureAll();




#endif