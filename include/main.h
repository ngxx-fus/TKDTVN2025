/// #pragma message("[include] main.h")

#include "Arduino.h"
#include <string.h>

#include "localHelperAndUtil.h"
#include "externalLibraryAndDriver.h"

#if (LIGHT_TICK_EN == 1)
    void TaskLightTick(void* pv){
        __entry("TaskLightTick()");
        /// Set-up LED for TaskLightTick
        gpio_config_t outPin = {
            .pin_bit_mask = __mask64(LIGHT_TICK_PIN),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };
        gpio_config(&outPin);
        /// TaskLightTick : Blink LED with period 200 tick
        __sys_log("[TaskLightTick] Info: GPIO=%d, TimeOn=%d TimeOff=%d", 
                    LIGHT_TICK_PIN, LIGHT_TICK_TIME_ON, LIGHT_TICK_TIME_OFF);
        const char pattern[] = LIGHT_TICK_CYCLIC_PATTERN;
        const int  patternSize = sizeof(LIGHT_TICK_CYCLIC_PATTERN);
        while(1){
            for(int i = 0; i < patternSize; ++i){
                if(pattern[i] == '0'){
                    GPIO.out_w1tc = __mask32(LIGHT_TICK_PIN);
                    vTaskDelay(LIGHT_TICK_TIME_ON);
                }
                if(pattern[i] == '1'){
                    GPIO.out_w1ts = __mask32(LIGHT_TICK_PIN);
                    vTaskDelay(LIGHT_TICK_TIME_OFF);
                }
            }
        }
        __exit("TaskLightTick()");
    }
#endif /// (LIGHT_TICK_EN == 1)

#if (SENSOR_HCSR04_EN == 1)
    /// @brief Task to handle HC-SR04 Ultrasonic Sensor measurements
    /// @param pv Task parameters (unused)
    void TaskHCSR04(void *pv){
        def returnValue = 0;
        
        /// Initialize Soft Timer for precise 950ms cycle
        espSoftTimer_t hcsr04Timer;
        espSoftTimerInit(&hcsr04Timer, HCSR04_MEASURE_INTERVAL); // 950ms = 950,000us

        /// Setup pins
        hcsr04Dev.T0 = hcsr04Dev.T1 = hcsr04Dev.T2 = hcsr04Dev.T3 = HCSR04_C_PIN;
        hcsr04Dev.E0 = HCSR04_0_PIN;    hcsr04Dev.E2 = HCSR04_2_PIN;
        hcsr04Dev.E1 = HCSR04_1_PIN;    hcsr04Dev.E3 = HCSR04_3_PIN;
        hcsr04Init();

        while(1){
            returnValue = hcsr04MeasureAll();
            
            if(returnValue != STATUS_OKE) 
                __sys_err("[TaskHCSR04] hcsr04MeasureAll(): %s", DEFAULT_RETURN_STATUS_STR(returnValue));
            
            __sys_log("[TaskHCSR04] F:%d R:%d B:%d L:%d", 
                        hcsr04Data.arr[0], hcsr04Data.arr[1],
                        hcsr04Data.arr[2], hcsr04Data.arr[3]);

            /// Wait for the remainder of the 950ms cycle (Precise Timing)
            /// This compensates for the execution time of measurements and logs
            __EST_WAIT_AND_RESET_EXEC(&hcsr04Timer, vTaskDelay(pdMS_TO_TICKS(10)));
        }
    }
#endif /// (SENSOR_HCSR04_EN == 1)

#if (SENSOR_HCSR04_EN == 1)
    /// @brief Task to handle HC-SR04 Ultrasonic Sensor measurements with precise timing
    /// @param pv Task parameters (unused)
    void TaskHCSR04(void *pv){
        def returnValue = 0;
        
        /// Calculate polling interval (5% of total interval)
        const uint32_t pollTick = pdMS_TO_TICKS((HCSR04_MEASURE_INTERVAL / 1000) * __EST_POOL_CHECK_PERCENT);

        /// Initialize Soft Timer
        espSoftTimer_t hcsr04Timer;
        espSoftTimerInit(&hcsr04Timer, HCSR04_MEASURE_INTERVAL);

        /// Setup pins
        hcsr04Dev.T0 = hcsr04Dev.T1 = hcsr04Dev.T2 = hcsr04Dev.T3 = HCSR04_C_PIN;
        hcsr04Dev.E0 = HCSR04_0_PIN;    hcsr04Dev.E2 = HCSR04_2_PIN;
        hcsr04Dev.E1 = HCSR04_1_PIN;    hcsr04Dev.E3 = HCSR04_3_PIN;
        hcsr04Init();

        while(1){
            /// 1. Execute Work
            returnValue = hcsr04MeasureAll();
            
            if(returnValue != STATUS_OKE) 
                __sys_err("[TaskHCSR04] hcsr04MeasureAll(): %s", DEFAULT_RETURN_STATUS_STR(returnValue));
            
            __sys_log("[TaskHCSR04] F:%d R:%d B:%d L:%d", 
                        hcsr04Data.arr[0], hcsr04Data.arr[1],
                        hcsr04Data.arr[2], hcsr04Data.arr[3]);

            /// 2. Wait logic: Pool check until timeout, then auto-reset
            while (!estIsTimeOutAndReset(&hcsr04Timer)) {
                /// Sleep for 5% interval
                vTaskDelay(pollTick);
            }
        }
    }
#endif /// (SENSOR_HCSR04_EN == 1)

#if (SENSOR_MPU6050_EN == 1)
    /// @brief Task to handle MPU6050 with precise timing
    /// @param pv Task parameters (unused)
    void TaskMPU6050(void *pv){
        __entry("TaskMPU6050()");

        /// Calculate polling interval (5% of total interval)
        const uint32_t pollTick = pdMS_TO_TICKS((MPU6050_MEASURE_INTERVAL / 1000) * __EST_POOL_CHECK_PERCENT);

        /// Initialize Soft Timer
        espSoftTimer_t mpuTimer;
        espSoftTimerInit(&mpuTimer, MPU6050_MEASURE_INTERVAL);

        // Init MPU6050
        if (mpu6050Init() != STATUS_OKE) {
            __sys_log("[TaskMPU6050] "  "MPU6050 connection failed!");
            vTaskDelete(NULL);
        } else {
            __sys_log("[TaskMPU6050] "  "MPU6050 connected successfully.");
        }
        
        // Main loop
        while (1) {
            /// 1. Execute Work
            mpu6050Measure();
            
            __sys_log("[TaskMPU6050] a[x: %d, y: %d, z: %d] g[x: %d, y: %d, z: %d]", 
                    mpuData.ax, mpuData.ay, mpuData.az, mpuData.gx, mpuData.gy, mpuData.gz);
            
            /// 2. Wait logic: Pool check until timeout, then auto-reset
            while (!estIsTimeOutAndReset(&mpuTimer)) {
                /// Sleep for 5% interval
                vTaskDelay(pollTick);
            }
        }

        __exit("TaskMPU6050()");
        vTaskDelete(NULL);
    }
#endif /// (SENSOR_MPU6050_EN == 1)

#if (SENSOR_ATGM336H_EN == 1)

/// --- ISR Variables ---
#if (GPS_PPS_EN == 1)
    static volatile bool gpsUpdateDataNow = false;
    
    void IRAM_ATTR gpsPpsIsr(){
        gpsUpdateDataNow = true; 
    }
#endif

// /// @brief Main Task for ATGM336H GPS Module with SoftTimer and PPS Bypass
// /// @param pv Task parameters (unused)
// void TaskATGM336H(void* pv){
//     __sys_log("[TaskATGM336H] Task Started. UART%d @%d", GPS_SERIAL_NUM, GPS_BAUD);

//     /// Calculate polling interval (5% of total interval)
//     const uint32_t pollTick = pdMS_TO_TICKS((GPS_MEASURE_INTERVAL / 1000) * __EST_POOL_CHECK_PERCENT);

//     /// 1. Initialize UART
//     gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

//     /// 2. Initialize PPS Pin if enabled
//     #if (GPS_PPS_EN == 1)
//         pinMode(GPS_PPS_PIN, INPUT);
//         attachInterrupt(digitalPinToInterrupt(GPS_PPS_PIN), gpsPpsIsr, RISING);
//     #endif

//     /// Initialize Soft Timer for Loop Cycle
//     espSoftTimer_t gpsCycleTimer;
//     espSoftTimerInit(&gpsCycleTimer, GPS_MEASURE_INTERVAL);

//     /// Initialize Soft Timer for Rx Timeout (5 seconds)
//     espSoftTimer_t rxTimeoutTimer;
//     espSoftTimerInit(&rxTimeoutTimer, 5000000); 

//     for(;;) {
//         /// 3. Parse NMEA data from UART (Work Phase)
//         while (gpsSerial.available()) {
//             gpsDevice.encode(gpsSerial.read());
//             /// Reset timeout timer whenever valid data stream is active
//             estResetTimer(&rxTimeoutTimer);
//         }

//         /// 3.1 Check for hardware timeout
//         if (estIsTimeOutAndReset(&rxTimeoutTimer)) {
//             __sys_err("[TaskATGM336H] Timeout! No data received for 5s. Check wiring RX/TX!");
//         }

//         /// 4. Check & Update Date/Time
//         if (gpsDevice.date.isUpdated() || gpsDevice.time.isUpdated()) {
//             if (gpsDevice.date.isValid() && gpsDevice.time.isValid()) {
//                 gpsData.year   = gpsDevice.date.year();
//                 gpsData.month  = gpsDevice.date.month();
//                 gpsData.day    = gpsDevice.date.day();
//                 gpsData.hour   = gpsDevice.time.hour();
//                 gpsData.minute = gpsDevice.time.minute();
//                 gpsData.second = gpsDevice.time.second();
                
//                 gpsSetUpdateFlag(GPS_UPDATA_DATE_TIME);
//             } else {
//                 __sys_err("[TaskATGM336H] Date/Time updated but INVALID.");
//             }
//         }

//         /// 5. Check & Update Location
//         if (gpsDevice.location.isUpdated()) {
//             if (gpsDevice.location.isValid()) {
//                 gpsData.latitude  = gpsDevice.location.lat();
//                 gpsData.longitude = gpsDevice.location.lng();
                
//                 if (gpsDevice.speed.isValid()) {
//                     gpsData.speed_kmh = gpsDevice.speed.kmph();
//                 }

//                 gpsSetUpdateFlag(GPS_UPDATA_LOCATION);
//             } else {
//                 __sys_err("[TaskATGM336H] Location updated but INVALID (No Fix).");
//             }
//         }

//         /// 6. Check & Update Others (Satellites)
//         if (gpsDevice.satellites.isUpdated()) {
//             if (gpsDevice.satellites.isValid()) {
//                 gpsData.sats = gpsDevice.satellites.value();
//                 gpsSetUpdateFlag(GPS_UPDATA_OTHERS);
//             } else {
//                  __sys_err("[TaskATGM336H] Sats count updated but INVALID.");
//             }
//         }

//         /// 7. Wait logic: Pool check until timeout OR PPS triggers
//         while (!estIsTimeOutAndReset(&gpsCycleTimer)) {
            
//             #if (GPS_PPS_EN == 1)
//                 /// Priority Bypass: If PPS triggers, break wait loop immediately to update data
//                 if (gpsUpdateDataNow) {
//                     gpsUpdateDataNow = false;
//                     /// Reset timer manually to start new cycle immediately from this point
//                     estResetTimer(&gpsCycleTimer);
//                     break; 
//                 }
//             #endif
            
//             /// Sleep for 5% interval
//             vTaskDelay(pollTick);
//         }
//     }
    
//     vTaskDelete(NULL);
// }

/// @brief Main Task for ATGM336H GPS Module with SoftTimer and PPS Bypass
/// @param pv Task parameters (unused)
void TaskATGM336H(void* pv){
    __sys_log("[TaskATGM336H] Task Started. UART%d @%d", GPS_SERIAL_NUM, GPS_BAUD);

    /// Calculate polling interval (5% of total interval)
    const uint32_t pollTick = pdMS_TO_TICKS((GPS_MEASURE_INTERVAL / 1000) * __EST_POOL_CHECK_PERCENT);

    /// 1. Initialize UART
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    /// 2. Initialize PPS Pin if enabled
    #if (GPS_PPS_EN == 1)
        pinMode(GPS_PPS_PIN, INPUT);
        attachInterrupt(digitalPinToInterrupt(GPS_PPS_PIN), gpsPpsIsr, RISING);
    #endif

    /// Initialize Soft Timer for Loop Cycle
    espSoftTimer_t gpsCycleTimer;
    espSoftTimerInit(&gpsCycleTimer, GPS_MEASURE_INTERVAL);

    /// Initialize Soft Timer for Rx Timeout (5 seconds)
    espSoftTimer_t rxTimeoutTimer;
    espSoftTimerInit(&rxTimeoutTimer, 5000000); 

    for(;;) {
        /// 3. Parse NMEA data from UART (Work Phase)
        while (gpsSerial.available()) {
            gpsDevice.encode(gpsSerial.read());
            /// Reset timeout timer whenever valid data stream is active
            estResetTimer(&rxTimeoutTimer);
        }

        /// 3.1 Check for hardware timeout
        if (estIsTimeOutAndReset(&rxTimeoutTimer)) {
            __sys_err("[TaskATGM336H] Timeout! No data received for 5s. Check wiring RX/TX!");
        }

        /// 4. Check & Update Date/Time
        if (gpsDevice.date.isUpdated() || gpsDevice.time.isUpdated()) {
            if (gpsDevice.date.isValid() && gpsDevice.time.isValid()) {
                gpsData.year   = gpsDevice.date.year();
                gpsData.month  = gpsDevice.date.month();
                gpsData.day    = gpsDevice.date.day();
                gpsData.hour   = gpsDevice.time.hour();
                gpsData.minute = gpsDevice.time.minute();
                gpsData.second = gpsDevice.time.second();
                
                gpsSetUpdateFlag(GPS_UPDATA_DATE_TIME);
            } else {
                __sys_err("[TaskATGM336H] Date/Time updated but INVALID.");
            }
        }

        /// 5. Check & Update Location
        if (gpsDevice.location.isUpdated()) {
            if (gpsDevice.location.isValid()) {
                gpsData.latitude  = gpsDevice.location.lat();
                gpsData.longitude = gpsDevice.location.lng();
                
                if (gpsDevice.speed.isValid()) {
                    gpsData.speed_kmh = gpsDevice.speed.kmph();
                }

                gpsSetUpdateFlag(GPS_UPDATA_LOCATION);
            } else {
                __sys_err("[TaskATGM336H] Location updated but INVALID (No Fix).");
            }
        }

        /// 6. Check & Update Others (Satellites)
        if (gpsDevice.satellites.isUpdated()) {
            if (gpsDevice.satellites.isValid()) {
                gpsData.sats = gpsDevice.satellites.value();
                gpsSetUpdateFlag(GPS_UPDATA_OTHERS);
            } else {
                 __sys_err("[TaskATGM336H] Sats count updated but INVALID.");
            }
        }

        // [FIXED] Convert floats to Strings before logging
        if (gpsData.latitude != 0.0) {
            __sys_log("[TaskATGM336H] %04d-%02d-%02d %02d:%02d:%02d | Lat: %s Lon: %s | Spd: %s km/h | Sats: %d",
                gpsData.year, gpsData.month, gpsData.day,
                gpsData.hour, gpsData.minute, gpsData.second,
                String(gpsData.latitude, 6).c_str(),
                String(gpsData.longitude, 6).c_str(),
                String(gpsData.speed_kmh, 2).c_str(),
                (int)gpsData.sats);
        } else {
             __sys_log("[TaskATGM336H] Searching for satellites... (Sats: %d)", (int)gpsDevice.satellites.value());
        }

        /// 7. Wait logic: Pool check until timeout OR PPS triggers
        while (!estIsTimeOutAndReset(&gpsCycleTimer)) {
            
            #if (GPS_PPS_EN == 1)
                /// Priority Bypass: If PPS triggers, break wait loop immediately to update data
                if (gpsUpdateDataNow) {
                    gpsUpdateDataNow = false;
                    /// Reset timer manually to start new cycle immediately from this point
                    estResetTimer(&gpsCycleTimer);
                    break; 
                }
            #endif
            
            /// Sleep for 5% interval
            vTaskDelay(pollTick);
        }
    }
    
    vTaskDelete(NULL);
}

#endif /// (SENSOR_ATGM336H_EN == 1)

#if (FIREBASE_SYNC_EN == 1) || (LAN_DATA_EXCHANGE_EN == 1) 
    void TaskWiFiFrequentlyCheck(void* pv){
        while(1){
            if(!WiFi.isConnected()){
                __sys_err("[TaskWiFiFrequentlyCheck] Wi-Fi is lost! Reconnect...");
                wfInit();
            }
            eldeUpdateSelfIP();
            vTaskDelay(pdMS_TO_TICKS(6*60*1000));
        }
    }

#endif /// (FIREBASE_SYNC_EN == 1) || (LAN_DATA_EXCHANGE_EN == 1)

#if (FIREBASE_SYNC_EN == 1)
    void TaskFirebaseSync(void* pv){
        __entry("TaskFirebaseSync()");
        espSoftTimer_t st;
        espSoftTimerInit(&st, FB_SYNC_DELAY);
        while(1){
            #if (SENSOR_HCSR04_EN == 1)
                if(fbUploadHCSR04Data()!=STATUS_OKE) __sys_log("[Task04] [fbUploadHCSR04Data] failed!");
            #endif /// (SENSOR_HCSR04_EN == 1)
            #if (SENSOR_MPU6050_EN == 1)
                if(fbUploadMPU6050Data()!=STATUS_OKE) __sys_log("[Task04] [fbUploadMPU6050Data] failed!");
            #endif /// (SENSOR_MPU6050_EN == 1)
            #if (SENSOR_ATGM336H_EN == 1)
                if(fbUploadATGM336HData()!=STATUS_OKE) __sys_log("[Task04] [fbUploadATGM336HData] failed!");
            #endif /// (SENSOR_ATGM336H_EN == 1)

            __EST_WAIT_AND_RESET_EXEC(&st, vTaskDelay(1));
        }
        __exit("TaskFirebaseSync()");
    }
#endif /// (FIREBASE_SYNC_EN == 1)

#if (LAN_DATA_EXCHANGE_EN == 1)

    /// @brief Task to exchange data with PC via UDP (TX ONLY)
    void TaskLANDataSend(void* pv) {
        /// 1. Wait for WiFi
        while (WiFi.status() != WL_CONNECTED) {
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        /// 2. Init Library
        if (eldeInit() != STATUS_OKE) {
            __sys_err("[TaskLANDataSend] ELDE Init Failed!");
        } else {
            __sys_log("[TaskLANDataSend] Ready. Target: %s:%d", broadcastHost.ip, broadcastHost.port.udp);
        }

        while (1) {
            // --- SENSOR DATA UPLOAD BLOCK ---

            #if (SENSOR_HCSR04_EN == 1)
                eldeSendFrame(ID_SENSOR_HCSR04, &hcsr04Data, sizeof(hcsr04Data), &broadcastHost);
            #endif /// (SENSOR_HCSR04_EN == 1)

            #if (SENSOR_MPU6050_EN == 1)
                eldeSendFrame(ID_SENSOR_MPU6050, &mpuData, sizeof(mpuData), &broadcastHost);
            #endif /// (SENSOR_MPU6050_EN == 1)

            #if (SENSOR_ATGM336H_EN == 1)
                eldeSendFrame(ID_SENSOR_ATGM336H, &gpsData, sizeof(gpsData), &broadcastHost);
            #endif /// (SENSOR_ATGM336H_EN == 1)

            /// Update rate 10Hz
            vTaskDelay(pdMS_TO_TICKS(500)); 
        }
    }

    /// @brief Task to receive data from PC (RX ONLY)
    void TaskLANDataReceive(void* pv){
        /// Buffer to store extracted payload (must be large enough)
        static uint8_t payloadBuf[256]; 
        /// Buffer to store Hex String (2 hex chars per byte + null terminator)
        static char hexStrBuf[513]; 

        while(1){
            /// Poll for data (Non-blocking)
            // eldePoll(); 

            /// Check if valid data arrived
            if(hasReceivedFrame(50)){
                uint8_t ID = 0;
                int DataSize = 0;

                /// Extract data to payloadBuf
                if(getFrameData(&ID, &DataSize, payloadBuf, sizeof(payloadBuf)) == STATUS_OKE){
                    
                    /// Reset hex string buffer
                    char* ptr = hexStrBuf;
                    
                    /// Convert Payload bytes to Hex String (e.g., 0A 1B FF)
                    for(int i = 0; i < DataSize; ++i){
                        /// Append byte as 2-digit Hex to string
                        ptr += sprintf(ptr, "%02X", payloadBuf[i]);
                    }
                    
                    /// Log result like Wireshark view
                    __sys_log("[TaskLANDataReceive] Received [ID=%d | Len=%d] DataHex: %s", ID, DataSize, hexStrBuf);
                }
                
            }

            vTaskDelay(pdMS_TO_TICKS(1)); /// Short delay to prevent Watchdog trigger
        }
    }

#endif /// (LAN_DATA_EXCHANGE_EN == 1)