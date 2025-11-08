#ifndef __PROJECT_CONFIG_H__
#define __PROJECT_CONFIG_H__

/// #pragma message("[include] projectConfig.h")

#include "stdint.h"

/// CONFIG | LOG //////////////////////////////////////////////////////////////////////////////////

#define SYS_LOG_L0_EN           1
#define SYS_LOG_L1_EN           0
#define SYS_LOG_L2_EN           0
#define SYS_LOG_err
#define SYS_LOG_L0_ENTRY        1
#define SYS_LOG_L0_EXIT         1
#define SYS_LOG_L1_ENTRY        0
#define SYS_LOG_L1_EXIT         0
#define SYS_LOG_L2_ENTRY        0
#define SYS_LOG_L2_EXIT         0

/// CONFIG | OTHERS ///////////////////////////////////////////////////////////////////////////////

#define LIGHT_TICK_EN           1
#define HM_STEP_MOTOR0_EN       1
#define HM_STEP_MOTOR1_EN       0


/// DEFINITIONS ///////////////////////////////////////////////////////////////////////////////////

#if (LIGHT_TICK_EN == 1)
    #define LIGHT_TICK_PIN          12
    #define LIGHT_TICK_TIME_ON      100
    #define LIGHT_TICK_TIME_OFF     400
#endif

#if (HM_STEP_MOTOR0_EN == 1)
    #define HM_STEP_MOTOR0_PIN0     5
    #define HM_STEP_MOTOR0_PIN1     4
    #define HM_STEP_MOTOR0_PIN2     3
    #define HM_STEP_MOTOR0_PIN3     2
#endif

#if (HM_STEP_MOTOR1_EN == 1)
    #define HM_STEP_MOTOR1_PIN0     9
    #define HM_STEP_MOTOR1_PIN1     8
    #define HM_STEP_MOTOR1_PIN2     7
    #define HM_STEP_MOTOR1_PIN3     6
#endif


#endif