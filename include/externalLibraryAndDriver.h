#ifndef __EXT_LIB_H__
#define __EXT_LIB_H__

/// #pragma message("[include] externalLibraryAndDriver.h")

#include "Arduino.h"
#include "projectConfig.h"

#if (SENSOR_MPU6050_EN == 1)
    #include "../lib/MPU6050Wrap/MPU6050Wrap.h"
#endif

#if (SENSOR_HCSR04_EN == 1)
    #include "../lib/HCSR04Wrap/HCSR04Wrap.h"
#endif

#if (SENSOR_ATGM336H_EN == 1)
    #include "../lib/atgm336hWrap/atgm336hWrap.h"
#endif

#if (FIREBASE_SYNC_EN == 1)
    #include "../lib/FirebaseWrap/FirebaseWrap.h"
#endif

#if (LAN_DATA_EXCHANGE_EN == 1)
    #include "../lib/espLANDataExchange/espLANDataExchange.h"
#endif

#endif