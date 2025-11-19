#include "../../include/projectConfig.h"
#if (FIREBASE_SYNC_EN == 1)

#include "../cReturnType/cReturnType.h"
#include "../espSoftTimer/espSoftTimer.h"

#include "FirebaseWrap.h"

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

/// @brief Initialize Wi-Fi with Dynamic Backoff Strategy (Non-blocking)
void wfInit(){
    /// Define wait intervals (microseconds)
    const int64_t SHORT_WAIT   = 2000000;   /// 2s
    const int64_t MEDIUM_WAIT  = 60000000;  /// 1 min
    const int64_t LONG_WAIT    = 600000000; /// 10 min

    int attemptCount = 0; 
    int64_t waitTimeUs = 0;
    espSoftTimer_t wfTimer;

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    /// Loop until connected
    while(WiFi.status() != WL_CONNECTED){
        attemptCount++;
        
        /// Calculate backoff time based on attempt count
        if(attemptCount < 10){
            waitTimeUs = SHORT_WAIT;    // Retries 1-10: 2s
        } else if(attemptCount < 20){
            waitTimeUs = MEDIUM_WAIT;   // Retries 11-20: 1 min
        } else {
            waitTimeUs = LONG_WAIT;     // Retries 20+: 10 min
        }

        __sys_log("[wfInit] Connecting to Wi-fi... (Attempt: %d, Wait: %ds)", 
                  attemptCount, (int)(waitTimeUs/1000000));
        
        /// Initialize timer
        espSoftTimerInit(&wfTimer, waitTimeUs);

        /// Safe Wait: Yields to OS while waiting
        __EST_WAIT_EXEC(&wfTimer, vTaskDelay(pdMS_TO_TICKS(100)));
        
        /// Hard Retry: Re-trigger connection logic if stuck too long
        if (attemptCount % 20 == 0) {
            WiFi.disconnect();
            WiFi.reconnect();
        }
    }
    
    __sys_log("[wfInit] Connected to Wi-fi!");
    __sys_log("[wfInit] IP Address: %s", WiFi.localIP().toString().c_str());
}

/// @brief Initialize Firebase with Dynamic Backoff Strategy (Non-blocking)
/// @details Handles cases where WiFi is connected but Internet is missing.
void fbInit() {
    __entry("fbInit()");

    /// Define wait intervals (microseconds)
    const int64_t FB_QUICK_WAIT = 2000000;   /// 2s
    const int64_t FB_SLOW_WAIT  = 10000000;  /// 10s
    const int64_t FB_LONG_WAIT  = 30000000;  /// 30s

    config.api_key = FB_API_KEY;
    config.database_url = FBRTDB_URL;
    auth.user.email = FB_USER_EMAIL;
    auth.user.password = FB_USER_PASSWORD;

    int attemptCount = 0;
    int64_t waitTimeUs = 0;
    espSoftTimer_t fbTimer;

    /// Initial configuration
    Firebase.begin(&config, &auth);

    /// Loop until Firebase is authenticated and ready
    while (!Firebase.ready()) {
        attemptCount++;

        __sys_log("[fbInit] Connecting to Firebase... (Attempt: %d)", attemptCount);
        
        /// Check specific error reasons if available
        if (firebaseData.httpCode() > 0 && firebaseData.httpCode() != 200) {
            __sys_log("[fbInit] HTTP Error: %d", firebaseData.httpCode());
        }
        if (firebaseData.errorReason().length() > 0) {
            __sys_log("[fbInit] Reason: %s", firebaseData.errorReason().c_str());
        }

        /// Calculate backoff time (Internet might be flaky)
        if (attemptCount < 5) {
            waitTimeUs = FB_QUICK_WAIT; // First 5 tries: 2s
        } else if (attemptCount < 15) {
            waitTimeUs = FB_SLOW_WAIT;  // Next 10 tries: 10s
        } else {
            waitTimeUs = FB_LONG_WAIT;  // Persistent failure: 30s
        }

        /// Initialize wait timer
        espSoftTimerInit(&fbTimer, waitTimeUs);

        /// Safe Wait: Yield to OS
        __EST_WAIT_EXEC(&fbTimer, vTaskDelay(pdMS_TO_TICKS(100)));

        /// Optional: Re-trigger begin if stuck very long (refresh token generation)
        if (attemptCount % 10 == 0) {
             Firebase.begin(&config, &auth);
        }
    }

    Firebase.reconnectNetwork(true);
    __sys_log("[fbInit] Connected to Firebase successfully!");
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
    /// @brief Upload ATGM336H GPS data to Firebase RTDB
    /// @return STATUS_OKE on success, STATUS_ERR on failure
    def fbUploadATGM336HData() {
        if (WiFi.status() != WL_CONNECTED) {
            return STATUS_ERR;
        }

        static FirebaseJson gpsJson;
        gpsJson.clear();

        // Location Data
        gpsJson.set("location_latitude", gpsData.latitude);
        gpsJson.set("location_longitude", gpsData.longitude);
        gpsJson.set("speed_kmh", gpsData.speed_kmh);
        gpsJson.set("sats", gpsData.sats);

        // GPS Timestamp
        gpsJson.set("date_year", gpsData.year);
        gpsJson.set("date_month", gpsData.month);
        gpsJson.set("date_day", gpsData.day);
        gpsJson.set("time_hh", gpsData.hour);
        gpsJson.set("time_mm", gpsData.minute);
        gpsJson.set("time_ss", gpsData.second);

        // System Timestamp (us)
        gpsJson.set("ts", esp_timer_get_time()); 

        // Attempt to update the node
        // Note: Ensure FBRTDB_ATGM336H_PATH is defined in projectConfig.h
        bool upload_ok = Firebase.updateNode(firebaseData, FBRTDB_ATGM336H_PATH, gpsJson);

        if (!upload_ok) {
            __sys_err("[fbUploadATGM336HData] Failed to sync GPS JSON data!");
            __sys_err("[fbUploadATGM336HData] Firebase Error: %s", firebaseData.errorReason().c_str());
            return STATUS_ERR;
        }

        return STATUS_OKE;
    }
#endif /// (SENSOR_ATGM336H_EN == 1)

#endif /// (FIREBASE_SYNC_EN == 1)