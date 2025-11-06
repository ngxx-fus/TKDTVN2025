#ifndef __MPU6050_WRAP_H__
#define __MPU6050_WRAP_H__


#include "stdint.h"
#include "Wire.h"
#include "MPU6050.h"

#include "../cReturnType/cReturnType.h"
#include "../espSerialWrap/espSerialWrap.h"

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

static inline def mpu6050Init(){
    __entry("mpu6050Init()");
    /// Initial I2C
    Wire.begin();
    vTaskDelay(pdMS_TO_TICKS(100));
    /// Initial MPU6050
    mpuDev.initialize();
    if (!mpuDev.testConnection()) {
        __exit("mpu6050Init() : ERR_INIT_FAILED");
        return ERR_INIT_FAILED;
    }
    __exit("mpu6050Init() : OKE");
    return OKE;
}

static inline void mpu6050Measure(mpu6050Data_t* result){
    mpuDev.getMotion6(
        &(result->ax), &(result->ay), &(result->az), 
        &(result->gx), &(result->gy), &(result->gz)
    );
}

static inline void mpu6050EncodeToTransmit(mpu6050Data_t* result, ){
    
}

static inline void mpu6050DecodeFromReceive(mpu6050Data_t* result){
    
}


#endif