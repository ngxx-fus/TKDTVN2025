/// @file    espSoftTimer.h
/// @brief   Non-blocking polling timer (Soft Timer) for ESP32
/// @author  Nguyen THanh Phu
/// @date    2025-11-19

#ifndef __ESP_SOFT_TIMER_H__
#define __ESP_SOFT_TIMER_H__

#include <stdint.h>
#include <stdbool.h>
#include <esp_timer.h>

/* --- Data Types --- */

/// @brief Structure to hold timer data
typedef struct espSoftTimer_t {
    int64_t __TimeInit;     ///< Timestamp when the timer started (us)
    int64_t __TimeInterval; ///< Duration to wait (us)
} espSoftTimer_t;

/* --- Function Prototypes & Implementation --- */

/// @brief   Initialize the timer (Stack version, no malloc).
/// @param   td Pointer to the timer struct.
/// @param   interval Timeout duration in microseconds.
static inline void espSoftTimerInit(espSoftTimer_t *td, int64_t interval_us) {
    if (td) {
        td->__TimeInterval = interval_us;
        td->__TimeInit = esp_timer_get_time();
    }
}

/// @brief   Check if the timer is timeout (now - init > interval).
/// @details Does NOT reset the timer.
/// @param   td Pointer to the timer struct.
/// @return  true if timeout, false if not.
static inline bool estIsTimeOut(const espSoftTimer_t *td) {
    if (!td) return false;
    return (esp_timer_get_time() - td->__TimeInit) >= td->__TimeInterval;
}

/// @brief   Check if timeout, AND reset if true.
/// @details Useful for periodic tasks (replaces isTimeOut1).
/// @param   td Pointer to the timer struct.
/// @return  true if timeout occurred (and timer was reset).
static inline bool estIsTimeOutAndReset(espSoftTimer_t *td) {
    if ((esp_timer_get_time() - td->__TimeInit) >= td->__TimeInterval) {
        // Reset start time to current time
        td->__TimeInit = esp_timer_get_time(); 
        return true;
    }
    return false;
}

/// @brief   Change the interval duration and restart.
/// @param   td Pointer to timer.
/// @param   interval New duration in us.
static inline void estSetInterval(espSoftTimer_t *td, int64_t interval) {
    if (td) {
        td->__TimeInterval = interval;
        td->__TimeInit = esp_timer_get_time(); // Restart required
    }
}

/// @brief   Manually reset the timer counter to 0.
static inline void estResetTimer(espSoftTimer_t *td) {
    if (td) {
        td->__TimeInit = esp_timer_get_time();
    }
}

/// @brief   Blocking wait loop until the timer expires.
/// @details Allows executing code (like vTaskDelay) while waiting.
/// @param   espSoftTimerPtr Pointer to initialized timer.
/// @param   ... Optional code to execute inside the loop (e.g., yield()).
#define __EST_WAIT_EXEC(espSoftTimerPtr, ...) \
    while (!estIsTimeOut(espSoftTimerPtr)) { \
        __VA_ARGS__; \
    }
/// @brief   Blocking wait loop until the timer expires then reset the soft timer.
/// @details Allows executing code (like vTaskDelay) while waiting.
/// @param   espSoftTimerPtr Pointer to initialized timer.
/// @param   ... Optional code to execute inside the loop (e.g., yield()).
#define __EST_WAIT_AND_RESET_EXEC(espSoftTimerPtr, ...) \
    while (!estIsTimeOutAndReset(espSoftTimerPtr)) { \
        __VA_ARGS__; \
    }

#define __EST_POOL_CHECK_PERCENT    0.05

#endif // __ESP_SOFT_TIMER_H__