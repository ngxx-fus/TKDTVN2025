#ifndef __LOG_ARDUINO_H__
#define __LOG_ARDUINO_H__

#include <Arduino.h>

// CONFIG: enable / disable log levels
#ifndef CB_LOG_L0_EN
    #define CB_LOG_L0_EN 1
#endif
#ifndef CB_LOG_L1_EN
    #define CB_LOG_L1_EN 0
#endif
#ifndef CB_LOG_L2_EN
    #define CB_LOG_L2_EN 0
#endif
#ifndef CB_LOG_ERR_EN
    #define CB_LOG_ERR_EN 1
#endif
#ifndef CB_LOG_L0_ENTRY
    #define CB_LOG_L0_ENTRY 1
#endif
#ifndef CB_LOG_L0_EXIT
    #define CB_LOG_L0_EXIT 1
#endif
#ifndef CB_LOG_L1_ENTRY
    #define CB_LOG_L1_ENTRY 0
#endif
#ifndef CB_LOG_L1_EXIT
    #define CB_LOG_L1_EXIT 0
#endif
#ifndef CB_LOG_L2_ENTRY
    #define CB_LOG_L2_ENTRY 0
#endif
#ifndef CB_LOG_L2_EXIT
    #define CB_LOG_L2_EXIT 0
#endif

// Core logging function (Arduino version)
template <typename... Args>
void __coreLog(const char *tag, Args... args)
{
    Serial.print('[');
    Serial.print(tag);
    Serial.print("] ");
    ((Serial.print(args)), ...);
    Serial.println();
}

// Log macros

#if (CB_LOG_L0_EN == 1)
    #define __log(...) __coreLog("log", __VA_ARGS__)
#else
    #define __log(...)
#endif

#if (CB_LOG_L1_EN == 1)
    #define __log1(...) __coreLog("log1", __VA_ARGS__)
#else
    #define __log1(...)
#endif

#if (CB_LOG_L2_EN == 1)
    #define __log2(...) __coreLog("log2", __VA_ARGS__)
#else
    #define __log2(...)
#endif

#if (CB_LOG_ERR_EN == 1)
    #define __err(...) __coreLog("err", __VA_ARGS__)
#else
    #define __err(...)
#endif

#if (CB_LOG_L0_ENTRY == 1)
    #define __entry(...) __coreLog(">>>", __VA_ARGS__)
#else
    #define __entry(...)
#endif

#if (CB_LOG_L0_EXIT == 1)
    #define __exit(...) __coreLog("<<<", __VA_ARGS__)
#else
    #define __exit(...)
#endif

#if (CB_LOG_L1_ENTRY == 1)
    #define __entry1(...) __coreLog(">>>1", __VA_ARGS__)
#else
    #define __entry1(...)
#endif

#if (CB_LOG_L1_EXIT == 1)
    #define __exit1(...) __coreLog("<<<1", __VA_ARGS__)
#else
    #define __exit1(...)
#endif

#if (CB_LOG_L2_ENTRY == 1)
    #define __entry2(...) __coreLog(">>>2", __VA_ARGS__)
#else
    #define __entry2(...)
#endif

#if (CB_LOG_L2_EXIT == 1)
    #define __exit2(...) __coreLog("<<<2", __VA_ARGS__)
#else
    #define __exit2(...)
#endif

#endif // __LOG_ARDUINO_H__
