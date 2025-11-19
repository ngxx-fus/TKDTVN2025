#include "../../include/projectConfig.h"
#if (FIREBASE_SYNC_EN == 1)

#include "FirebaseWrap.h"
#include "cReturnType.h"

#include "WiFi.h"
#include "FirebaseESP32.h"
#include "FirebaseJson.h"

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
    /// 
    int attemptCount = 0; 
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    do{
        attemptCount = (attemptCount+1)%20;
        __sys_log("[wfInit] Connecting to Wi-fi ... (%d)", attemptCount);
        if(attemptCount < 3){
            delay(2000);
        }else
        if(attemptCount < 10){
            delay(2000);
        }else
        if(attemptCount < 15){
            delay(60000);
        }else{
            delay(600000);
        }

    }while(WiFi.status() != WL_CONNECTED);
    __sys_log("[wfInit] Connected to Wi-fi!");
}

void fbInit() {
    __entry("fbInit()");
    config.api_key = FB_API_KEY;
    config.database_url = FBRTDB_URL;
    auth.user.email = FB_USER_EMAIL;
    auth.user.password = FB_USER_PASSWORD;
    do {
        __sys_log("[fbInit]  Connecting...");
        Firebase.begin(&config, &auth);
        if (firebaseData.httpCode() != 200) {
            __sys_log("[fbInit]  HTTP code: %d", firebaseData.httpCode());
        }else{
            __sys_log("[fbInit]  HTTP code: %d", firebaseData.httpCode());
        }
        if (firebaseData.errorReason().length() > 0) {
            __sys_log("Error: %s", firebaseData.errorReason().c_str());
        }
    }while(!Firebase.ready());
    Firebase.reconnectNetwork(true);
    __sys_log("[fbInit] Connected to Firebase!");
}

#if (SENSOR_MPU6050_EN == 1)
    def fbUploadMPU6050Data() {
        if (WiFi.status() != WL_CONNECTED) {
            return STATUS_ERR;
        }
        static FirebaseJson mpuJson;
        mpuJson.clear();
        mpuJson.set("ax", mpuData.ax);
        mpuJson.set("ay", mpuData.ay);
        mpuJson.set("az", mpuData.az);
        mpuJson.set("gx", mpuData.gx);
        mpuJson.set("gy", mpuData.gy);
        mpuJson.set("gz", mpuData.gz);
        mpuJson.set("ts", esp_timer_get_time()); 

        bool upload_ok = Firebase.updateNode(firebaseData, FBRTDB_MPU6050_PATH, mpuJson);

        if (!upload_ok) {
            __sys_err("[fbUploadMPU6050Data] Failed to sync MPU6050 JSON data!");
            __sys_err("[fbUploadMPU6050Data] Firebase Error: %s", firebaseData.errorReason().c_str());
            return STATUS_ERR;
        }

        return STATUS_OKE;
    }
#endif /// (SENSOR_MPU6050_EN == 1)

#if (SENSOR_HCSR04_EN == 1)
    def fbUploadHCSR04Data() {
        if (WiFi.status() != WL_CONNECTED) {
            return STATUS_ERR;
        }
        static FirebaseJson hcsr04Json;
        hcsr04Json.clear();
        hcsr04Json.set("front", hcsr04Data.__0);
        hcsr04Json.set("right", hcsr04Data.__1);
        hcsr04Json.set("back", hcsr04Data.__2);
        hcsr04Json.set("left", hcsr04Data.__3);
        hcsr04Json.set("ts", esp_timer_get_time()); 

        bool upload_ok = Firebase.updateNode(firebaseData, FBRTDB_HCSR04_PATH, hcsr04Json);

        if (!upload_ok) {
            __sys_err("[fbUploadMPU6050Data] Failed to sync MPU6050 JSON data!");
            __sys_err("[fbUploadMPU6050Data] Firebase Error: %s", firebaseData.errorReason().c_str());
            return STATUS_ERR;
        }

        return STATUS_OKE;
    }
#endif /// (SENSOR_HCSR04_EN == 1)

#if (SENSOR_ATGM336H_EN == 1)
    def fbUploadATGM336HData(){
     
        return STATUS_ERR;
    }
#endif /// (SENSOR_ATGM336H_EN == 1)

#endif /// (FIREBASE_SYNC_EN == 1)