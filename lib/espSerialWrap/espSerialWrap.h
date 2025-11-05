#ifndef __LOG_ARDUINO_H__
#define __LOG_ARDUINO_H__

#include <Arduino.h>

#include "../../include/projectConfig.h"

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

// Log macros

#if (SYS_LOG_L0_EN == 1)
    #define __log(fmt, ...)          ets_printf("[%lld] [log] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __log(fmt, ...)
#endif

#if (SYS_LOG_L1_EN == 1)
    #define __log1(fmt, ...)         ets_printf("[%lld] [log1] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __log1(fmt, ...)
#endif

#if (SYS_LOG_L2_EN == 1)
    #define __log2(fmt, ...)         ets_printf("[%lld] [log2] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __log2(fmt, ...)
#endif

#if (SYS_LOG_ERR_EN == 1)
    #define __err(fmt, ...)          ets_printf("[%lld] [err] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __err(fmt, ...)
#endif

#if (SYS_LOG_L0_ENTRY == 1)
    #define __entry(fmt, ...)        ets_printf("[%lld] [>>>] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __entry(fmt, ...)
#endif

#if (SYS_LOG_L0_EXIT == 1)
    #define __exit(fmt, ...)         ets_printf("[%lld] [<<<] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __exit(fmt, ...)
#endif

#if (SYS_LOG_L1_ENTRY == 1)
    #define __entry1(fmt, ...)       ets_printf("[%lld] [>>>] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __entry1(fmt, ...)
#endif

#if (SYS_LOG_L1_EXIT == 1)
    #define __exit1(fmt, ...)        ets_printf("[%lld] [<<<] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __exit1(fmt, ...)
#endif

#if (SYS_LOG_L2_ENTRY == 1)
    #define __entry2(fmt, ...)       ets_printf("[%lld] [>>>] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __entry2(fmt, ...)
#endif

#if (SYS_LOG_L2_EXIT == 1)
    #define __exit2(fmt, ...)        ets_printf("[%lld] [<<<] " fmt "\n", esp_timer_get_time(), ##__VA_ARGS__)
#else
    #define __exit2(fmt, ...)
#endif

#endif // __LOG_ARDUINO_H__
