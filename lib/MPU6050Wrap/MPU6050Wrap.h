#ifndef __MPU6050_WRAP_H__
#define __MPU6050_WRAP_H__

/// #pragma message("[include] MPU6050Wrap.h")

#include "stdint.h"                                     /// For uint8_t, int16_t, ...
#include "../../include/projectConfig.h"                /// For *_EN, ... macros
#include "../cReturnType/cReturnType.h"                 /// For def, STATUS_OKE, STATUS_ERR*
#include "../espSerialWrap/espSerialWrap.h"             /// For __sys_log, __entry, __exit,...
#include "../freeRTOSWrap/freeRTOSWrap.h"

#include "Wire.h"                                       /// For I2C Hardware
#include "MPU6050.h"                                    /// For MPU6050 sensor

/// SDA: GPIO21
/// SCL: GPIO22

typedef union mpu6050Data_t{
    struct {
        int16_t ax;
        int16_t ay;
        int16_t az;
        int16_t gx;
        int16_t gy;
        int16_t gz;
    };
    uint8_t arr[sizeof(int16_t) * 6];
}   mpu6050Data_t;

extern MPU6050 mpuDev;
extern mpu6050Data_t mpuData;

static inline def mpu6050Init(){
    __entry("mpu6050Init()");
    /// Initial I2C
    Wire.begin();
    vTaskDelay(pdMS_TO_TICKS(100));
    /// Initial MPU6050
    mpuDev.initialize();
    if (!mpuDev.testConnection()) {
        __exit("mpu6050Init() : SYS_STATUS_ERR__INIT_FAILED");
        return STATUS_ERR_INIT_FAILED;
    }
    __exit("mpu6050Init() : OKE");
    return STATUS_OKE;
}

static inline void mpu6050Measure(){
    mpuDev.getMotion6(
        &(mpuData.ax), &(mpuData.ay), &(mpuData.az), 
        &(mpuData.gx), &(mpuData.gy), &(mpuData.gz)
    );
}

// static inline void mpu6050EncodeToTransmit(mpu6050Data_t* result, ){
    // 
// }

// static inline void mpu6050DecodeFromReceive(mpu6050Data_t* result){
    // 
// }


#endif