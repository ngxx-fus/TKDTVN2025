#ifndef __SERIAL_WRAP_H__
#define __SERIAL_WRAP_H__

#include <Arduino.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>

/// AUTO CONFIG LOG  //////////////////////////////////////////////////////////////////////////////

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

/// CORE LOG FUNCTION /////////////////////////////////////////////////////////////////////////////

void __coreLog(const char* fmt, ...);

// AVR không có esp_timer_get_time
#define __TS micros()    // uint32_t

/// LOG MACRO /////////////////////////////////////////////////////////////////////////////////////

#if (SYS_LOG_L0_EN == 1)
    #define __sys_log(fmt, ...)  __coreLog("[%lu] [log] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __sys_log(fmt, ...)
#endif

#if (SYS_LOG_L1_EN == 1)
    #define __sys_log1(fmt, ...) __coreLog("[%lu] [log1] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __sys_log1(fmt, ...)
#endif

#if (SYS_LOG_L2_EN == 1)
    #define __sys_log2(fmt, ...) __coreLog("[%lu] [log2] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __sys_log2(fmt, ...)
#endif

#if (SYS_LOG_ERR_EN == 1)
    #define __sys_err(fmt, ...)  __coreLog("[%lu] [err] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __sys_err(fmt, ...)
#endif

#if (SYS_LOG_L0_ENTRY == 1)
    #define __entry(fmt, ...)    __coreLog("[%lu] [>>>] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __entry(fmt, ...)
#endif

#if (SYS_LOG_L0_EXIT == 1)
    #define __exit(fmt, ...)     __coreLog("[%lu] [<<<] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __exit(fmt, ...)
#endif

#if (SYS_LOG_L1_ENTRY == 1)
    #define __entry1(fmt, ...)   __coreLog("[%lu] [>>>] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __entry1(fmt, ...)
#endif

#if (SYS_LOG_L1_EXIT == 1)
    #define __exit1(fmt, ...)    __coreLog("[%lu] [<<<] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __exit1(fmt, ...)
#endif

#if (SYS_LOG_L2_ENTRY == 1)
    #define __entry2(fmt, ...)   __coreLog("[%lu] [>>>] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __entry2(fmt, ...)
#endif

#if (SYS_LOG_L2_EXIT == 1)
    #define __exit2(fmt, ...)    __coreLog("[%lu] [<<<] " fmt "\n", __TS, ##__VA_ARGS__)
#else
    #define __exit2(fmt, ...)
#endif

#endif // __SERIAL_WRAP_H__
