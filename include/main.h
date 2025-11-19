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
    // void taskWiFiFrequencyCheck(void* pv){
    //     eldeUpdateSelfIP();
    // }

#endif 

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

/// @brief Task to exchange data with PC via UDP
/// @param pv Task parameters
void TaskLANDataExchange(void* pv) {
    espLANHost_t pcHost;
    pcHost.hostName = "WindowsHost";
    pcHost.ip       = "192.168.2.165"; // Update your PC IP here
    pcHost.port.udp = 4210;
    pcHost.port.tcp = 4211;

    /// 1. Wait for WiFi connection
    while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    /// 2. Initialize LAN Exchange Library
    if (eldeInit() != STATUS_OKE) {
        __sys_err("[TaskLANDataExchange] ELDE Init Failed!");
    } else {
        __sys_log("[TaskLANDataExchange] ELDE Init Success! Listening on %d", thisESP.port.udp);
    }

    /// Define Protocol Constants
    const char* FRAME_START = "ATGM336H_START";
    const char* FRAME_STOP  = "ATGM336H_STOP";
    const size_t HEADER_LEN = strlen(FRAME_START);
    const size_t DATA_LEN   = sizeof(atgm336hData_t); // 20 bytes
    const size_t FOOTER_LEN = strlen(FRAME_STOP);
    
    /// Buffer size = Header + Data + Footer
    uint8_t txBuffer[64]; 

    while (1) {
        // --- SEND DATA (TX) ---
        
        /// Check if GPS struct size is correct (Safety check)
        if (DATA_LEN == 20) {
            /// 1. Copy Header
            memcpy(txBuffer, FRAME_START, HEADER_LEN);
            
            /// 2. Copy Raw Struct Data (from global gpsData)
            memcpy(txBuffer + HEADER_LEN, gpsData.arr, DATA_LEN);
            
            /// 3. Copy Footer
            memcpy(txBuffer + HEADER_LEN + DATA_LEN, FRAME_STOP, FOOTER_LEN);

            /// 4. Send UDP Packet
            def ret = espUDPSendByteArr(&pcHost, txBuffer, HEADER_LEN + DATA_LEN + FOOTER_LEN);
            
            if (ret == STATUS_OKE) {
                // __sys_log("[TaskLANDataExchange] Sent GPS Frame (%d bytes)", HEADER_LEN + DATA_LEN + FOOTER_LEN);
            }
        }

        // --- RECEIVE DATA (RX) ---
        eldePoll(); 

        uint8_t rxBuf[128];
        def len = espUDPReceiveByteArr(rxBuf, 128);
        if (len > 0 && len != STATUS_ERR) {
            rxBuf[len] = 0; // Null-terminate
            __sys_log("[TaskLANDataExchange] Recv from PC: %s", (char*)rxBuf);
        }

        /// Delay 100ms (10Hz update rate)
        vTaskDelay(pdMS_TO_TICKS(100)); 
    }
}
#endif /// (LAN_DATA_EXCHANGE_EN == 1)