#pragma once
#include <stdint.h>

// Thử include projectConfig.h nếu project có cung cấp
#if defined(__has_include)
#  if __has_include(<projectConfig.h>)
#    include <projectConfig.h>
#  endif
#endif

// ===== Defaults (project có thể override bằng -D trong platformio.ini) =====
#ifndef HCSR04_WAIT_FOR_HIGH
#define HCSR04_WAIT_FOR_HIGH    3000    // us
#endif
#ifndef HCSR04_WAIT_FOR_LOW
#define HCSR04_WAIT_FOR_LOW     40000   // us
#endif
#ifndef HCSR04_WAIT_BEFORE_CONT
#define HCSR04_WAIT_BEFORE_CONT 60      // ms
#endif
#ifndef HCSR04_SERSOR_NUM
#define HCSR04_SERSOR_NUM 4
#endif

// các include còn lại của lib (đừng dùng đường dẫn ../)
// ví dụ:
#include <cReturnType.h>
#include <espSerialWrap.h>
#include <cBitWiseMacro.h>
#include <cArithmeticMacro.h>
#include <freeRTOSWrap.h>


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
DEFAULT_RETURN_STATUS hcsr04Measure(int i);
DEFAULT_RETURN_STATUS hcsr04MeasureAll();



