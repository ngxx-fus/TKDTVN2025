#ifndef __LOG_ARDUINO_H__
#define __LOG_ARDUINO_H__

/// #pragma message("[include] espSerialWrap.h")

#include <Arduino.h>

#include "../../include/projectConfig.h"

#ifndef SYS_SEMAPHORE_LOG_EN
    #define SYS_SEMAPHORE_LOG_EN 1
#endif

#if (SYS_SEMAPHORE_LOG_EN == 1)
    /// For mutex lock
    #include "freertos/FreeRTOS.h"
    #include "freertos/semphr.h"
#endif

// CONFIG: enable / disable log levels
#ifndef SYS_LOG_L0_EN
    #define SYS_LOG_L0_EN 1
#endif
#ifndef SYS_LOG_L1_EN
    #define SYS_LOG_L1_EN 0
#endif
#ifndef SYS_LOG_L2_EN
    #define SYS_LOG_L2_EN 0
#endif
#ifndef SYS_LOG_ERR_EN
    #define SYS_LOG_ERR_EN 1
#endif
#ifndef SYS_LOG_L0_ENTRY
    #define SYS_LOG_L0_ENTRY 1
#endif
#ifndef SYS_LOG_L0_EXIT
    #define SYS_LOG_L0_EXIT 1
#endif
#ifndef SYS_LOG_L1_ENTRY
    #define SYS_LOG_L1_ENTRY 0
#endif
#ifndef SYS_LOG_L1_EXIT
    #define SYS_LOG_L1_EXIT 0
#endif
#ifndef SYS_LOG_L2_ENTRY
    #define SYS_LOG_L2_ENTRY 0
#endif
#ifndef SYS_LOG_L2_EXIT
    #define SYS_LOG_L2_EXIT 0
#endif

#if (SYS_SEMAPHORE_LOG_EN == 1)
    extern SemaphoreHandle_t logMutex;
    
    inline void semaphoreLogInit(){
        if (logMutex == NULL)
        logMutex = xSemaphoreCreateMutex();
    }

    #define __coreLog(...)              do{ if (logMutex) xSemaphoreTake(logMutex, portMAX_DELAY); \
                                        ets_printf(__VA_ARGS__); if (logMutex) xSemaphoreGive(logMutex);} while(0);
#else
    #define __coreLog(...)              ets_printf(__VA_ARGS__);
#endif

// Log macros

#if (SYS_LOG_L0_EN == 1)
    #define __sys_log(fmt, ...)          __coreLog("[%lld] [log] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __sys_log(fmt, ...)
#endif

#if (SYS_LOG_L1_EN == 1)
    #define __sys_log1(fmt, ...)         __coreLog("[%lld] [log1] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __sys_log1(fmt, ...)
#endif

#if (SYS_LOG_L2_EN == 1)
    #define __sys_log2(fmt, ...)         __coreLog("[%lld] [log2] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __sys_log2(fmt, ...)
#endif

#if (SYS_LOG_ERR_EN == 1)
    #define __sys_err(fmt, ...)          __coreLog("[%lld] [err] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __sys_err(fmt, ...)
#endif

#if (SYS_LOG_L0_ENTRY == 1)
    #define __entry(fmt, ...)        __coreLog("[%lld] [>>>] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __entry(fmt, ...)
#endif

#if (SYS_LOG_L0_EXIT == 1)
    #define __exit(fmt, ...)         __coreLog("[%lld] [<<<] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __exit(fmt, ...)
#endif

#if (SYS_LOG_L1_ENTRY == 1)
    #define __entry1(fmt, ...)       __coreLog("[%lld] [>>>] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __entry1(fmt, ...)
#endif

#if (SYS_LOG_L1_EXIT == 1)
    #define __exit1(fmt, ...)        __coreLog("[%lld] [<<<] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __exit1(fmt, ...)
#endif

#if (SYS_LOG_L2_ENTRY == 1)
    #define __entry2(fmt, ...)       __coreLog("[%lld] [>>>] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __entry2(fmt, ...)
#endif

#if (SYS_LOG_L2_EXIT == 1)
    #define __exit2(fmt, ...)        __coreLog("[%lld] [<<<] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __exit2(fmt, ...)
#endif

#endif // __LOG_ARDUINO_H__
