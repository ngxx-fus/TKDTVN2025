#ifndef __FIREBASE_WRAP_H__
#define __FIREBASE_WRAP_H__

/// #pragma message("[include] FirebaseWrap.h")
#include "../../include/projectConfig.h"

#if (FIREBASE_SYNC_EN == 1)

#include <stdint.h>
#include "../espSerialWrap/espSerialWrap.h"
#include "../espSoftTimer/espSoftTimer.h"
#include "../cReturnType/cReturnType.h"
#include "../MPU6050Wrap/MPU6050Wrap.h"
#include "../HCSR04Wrap/HCSR04Wrap.h"
#include "../atgm336hWrap/atgm336hWrap.h"

/// TODO: 

#ifndef FBRTDB_URL
    #error ("You must define <FBRTDB_URL> macro to use this wraper-header")
#endif
#ifndef FB_API_KEY
    #error ("You must define <FB_USER_EMAIL> macro to use this wraper-header")
#endif
#ifndef FB_USER_EMAIL
    #error ("You must define <FB_USER_EMAIL> macro to use this wraper-header")
#endif
#ifndef FB_USER_PASSWORD
    #error ("You must define <FB_USER_PASSWORD> macro to use this wraper-header")
#endif

#ifndef WIFI_SSID
    #error ("You must define <WIFI_SSID> macro to use this wraper-header")
#endif
#ifndef WIFI_PASSWORD
    #error ("You must define <WIFI_PASSWORD> macro to use this wraper-header")
#endif

#ifndef FBRTDB_ROOT_PATH
    #define FBRTDB_ROOT_PATH "/"
#endif

#ifndef FBRTDB_MPU6050_PATH
    #define FBRTDB_MPU6050_PATH FBRTDB_ROOT_PATH "mpu6050"
#endif


#include "WiFi.h"
#include "FirebaseESP32.h"


extern FirebaseData firebaseData;
extern FirebaseAuth auth;
extern FirebaseConfig config;

def wfIsConnected();
void wfInit();
void fbInit();

def fbUploadMPU6050Data();
def fbUploadHCSR04Data();
def fbUploadATGM336HData();


#endif /// FIREBASE_SYNC_EN == 1
#endif /// __FIREBASE_WRAP_H__