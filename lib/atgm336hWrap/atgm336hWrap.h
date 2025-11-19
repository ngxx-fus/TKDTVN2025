#include "../../include/projectConfig.h"
#if (SENSOR_ATGM336H_EN == 1)
#ifndef __ATGM336H_WRAP_H__
#define __ATGM336H_WRAP_H__

#include <TinyGPSPlus.h>

#if 1 /// Default pin
    #ifndef SENSOR_GPS_EN
        #define SENSOR_GPS_EN 1
    #endif
    #ifndef GPS_RX_PIN
        #define GPS_RX_PIN 33
    #endif
    #ifndef GPS_TX_PIN
        #define GPS_TX_PIN 32
    #endif
    #ifndef GPS_BAUD
        #define GPS_BAUD 115200
    #endif
    #ifndef GPS_PPS_EN
        #define GPS_PPS_EN 0
    #endif
    #ifndef GPS_PPS_PIN
        #define GPS_PPS_PIN 63
    #endif
#endif

extern HardwareSerial   gpsSerial(1);
extern TinyGPSPlus      gpsDevice;

void gpsInit();
void TaskGPS(void* pv);

#endif /// __ATGM336H_WRAP_H__
#endif /// (SENSOR_ATGM336H_EN == 1)