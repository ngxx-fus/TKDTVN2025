#ifndef __EXT_LIB_H__
#define __EXT_LIB_H__

#include <Arduino.h>
#include "projectConfig.h" // Giả sử file này ở cùng thư mục

#if (SENSOR_MPU6050_EN == 1)
    // Sửa đường dẫn: Dùng <> và bỏ đường dẫn
    #include <MPU6050Wrap.h>
#endif

#if (SENSOR_HCSR04_EN == 1)
    // Sửa đường dẫn: Dùng <> và bỏ đường dẫn
    #include <HCSR04Wrap.h>
#endif

#endif // __EXT_LIB_H__