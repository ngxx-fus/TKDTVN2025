#ifndef __EXT_LIB_H__
#define __EXT_LIB_H__

/// #pragma message("[include] extLibraryAndDriver.h")

#include "Arduino.h"
#include "projectConfig.h"

#if (SENSOR_MPU6050_EN == 1)
    #include "../lib/MPU6050Wrap/MPU6050Wrap.h"
#endif

#if (SENSOR_HCSR04_EN == 1)
    #include "../lib/HCSR04Wrap/HCSR04Wrap.h"
#endif

#endif