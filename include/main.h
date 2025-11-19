/// #pragma message("[include] main.h")

#include "Arduino.h"

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

#if (SENSOR_HCSR04_EN == 1)
    void TaskHCSR04(void *pv){
        def returnValue = 0;
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
            vTaskDelay(pdMS_TO_TICKS(950));
        }
    }
#endif /// (SENSOR_HCSR04_EN == 1)

#if (SENSOR_MPU6050_EN == 1)
    void TaskMPU6050(void *pv){
        __entry("TaskMPU6050()");

        // Init MPU6050
        if (mpu6050Init() != STATUS_OKE) {
            __sys_log("[TaskMPU6050] "  "MPU6050 connection failed!");
            vTaskDelete(NULL);
        } else {
            __sys_log("[TaskMPU6050] "  "MPU6050 connected successfully.");
        }
        // Main loop
        while (1) {
            mpu6050Measure();
            __sys_log("[TaskMPU6050] a[x: %d, y: %d, z: %d] g[x: %d, y: %d, z: %d]", 
                    mpuData.ax, mpuData.ay, mpuData.az, mpuData.gx, mpuData.gy, mpuData.gz);
            vTaskDelay(pdMS_TO_TICKS(350));
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

/// @brief Main Task for ATGM336H GPS Module
/// @param pv Task parameters (unused)
void TaskATGM336H(void* pv){
    __sys_log("[TaskATGM336H] Task Started. UART%d @%d", GPS_SERIAL_NUM, GPS_BAUD);

    /// 1. Initialize UART
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

    /// 2. Initialize PPS Pin if enabled
    #if (GPS_PPS_EN == 1)
        pinMode(GPS_PPS_PIN, INPUT);
        attachInterrupt(digitalPinToInterrupt(GPS_PPS_PIN), gpsPpsIsr, RISING);
    #endif

    uint32_t lastRxTime = millis();

    for(;;) {
        /// 3. Parse NMEA data from UART
        while (gpsSerial.available()) {
            gpsDevice.encode(gpsSerial.read());
            lastRxTime = millis(); // Update RX timestamp
        }

        /// 3.1 Check for hardware timeout (No data for 5 seconds)
        if (millis() - lastRxTime > 5000) {
            __sys_err("[TaskATGM336H] Timeout! No data received for 5s. Check wiring RX/TX!");
            lastRxTime = millis(); // Reset to avoid spamming error log
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
                /// Log error if NMEA sentence received but data is garbage
                __sys_err("[TaskATGM336H] Date/Time updated but INVALID.");
            }
        }

        /// 5. Check & Update Location
        if (gpsDevice.location.isUpdated()) {
            if (gpsDevice.location.isValid()) {
                gpsData.latitude  = gpsDevice.location.lat();
                gpsData.longitude = gpsDevice.location.lng();
                
                /// Update speed here as it usually comes with location
                if (gpsDevice.speed.isValid()) {
                    gpsData.speed_kmh = gpsDevice.speed.kmph();
                }

                gpsSetUpdateFlag(GPS_UPDATA_LOCATION);
            } else {
                /// Log error, usually means 'V' (Void) status in NMEA (No GPS Fix yet)
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

        /// 7. Handle PPS (Optional logic from old task)
        #if (GPS_PPS_EN == 1)
            if (gpsUpdateDataNow) {
                gpsUpdateDataNow = false;
                /// PPS handling logic here (if needed)
            }
        #endif

        /// 8. Yield to other tasks
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    vTaskDelete(NULL);
}

#endif /// (SENSOR_ATGM336H_EN == 1)

