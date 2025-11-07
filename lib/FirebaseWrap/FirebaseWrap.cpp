#include "FirebaseWrap.h"

#include "WiFi.h"
#include "FirebaseESP32.h"
#include <cReturnType.h>

FirebaseData firebaseData;
FirebaseAuth auth;
FirebaseConfig config;

def wfIsConnected(){
    if(WiFi.status() != WL_CONNECTED){
        return STATUS_ERR;
    }
    return STATUS_OKE;
}

void wfInit(){
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
        do{
            __sys_log("[wfInit] Connecting to Wi-fi...");
            delay(2000);
        }while(WiFi.status() != WL_CONNECTED);
    __sys_log("[wfInit] Connected to Wi-fi!");
}

void fbInit() {
    __entry("fbInit()");
    config.api_key = FBRTDB_URL;
    config.database_url = FBRTDB_URL;
    auth.user.email = FB_USER_EMAIL;
    auth.user.password = FB_USER_PASSWORD;
    do {
        __sys_log("[fbInit]  Connecting...");
        Firebase.begin(&config, &auth);
        if (firebaseData.httpCode() != 200) {
            __sys_log("HTTP code: %d", firebaseData.httpCode());
        }
        if (firebaseData.errorReason().length() > 0) {
            __sys_log("Error: %s", firebaseData.errorReason().c_str());
        }
    }while(!Firebase.ready());
    Firebase.reconnectNetwork(true);
    __sys_log("[fbInit] Connected to Firebase!");
}

def fbUploadMPU6050Data(){

    if (WiFi.status() != WL_CONNECTED) return STATUS_ERR;

    uint8_t upload =
        Firebase.setString(firebaseData, FBRTDB_MPU6050_PATH "/ax", String(mpuData.ax)) &&
        Firebase.setString(firebaseData, FBRTDB_MPU6050_PATH "/ay", String(mpuData.ay)) &&
        Firebase.setString(firebaseData, FBRTDB_MPU6050_PATH "/az", String(mpuData.az)) &&
        Firebase.setString(firebaseData, FBRTDB_MPU6050_PATH "/gx", String(mpuData.gx)) &&
        Firebase.setString(firebaseData, FBRTDB_MPU6050_PATH "/gy", String(mpuData.gy)) &&
        Firebase.setString(firebaseData, FBRTDB_MPU6050_PATH "/gz", String(mpuData.gz));

    if (upload == 0) {
        __sys_err("[fbUploadMPU6050Data] Failed to sync MPU6050 data to Firebase!");
        return STATUS_ERR;
    }

    // Optional: đọc lại để xác nhận
    // Firebase.getString(firebaseData, FBRTDB_MPU6050_PATH "/ax");
    return STATUS_OKE;
}
