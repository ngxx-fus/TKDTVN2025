/* ===================================================
 * FILE: src/gps_task.cpp
 * (Đây là code cho "yourTask")
 * ===================================================*/
#include "main.h"       // macro log, semaphore
#include "gps_task.h"
#include <TinyGPSPlus.h>

static HardwareSerial GPSSerial(2);  // static để tránh đụng symbol
static TinyGPSPlus gps;

#if (GPS_USE_PPS == 1)
static volatile bool ppsTick = false;
void IRAM_ATTR gpsPpsIsr(){ ppsTick = true; }
#endif



/**
 * @brief In tiêu đề CSV (Dùng __coreLog)
 */
static void printCsvHeader() {
    if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        // Dùng __coreLog (từ espSerialWrap.h) để in
        __coreLog("utc_date,utc_time,lat,lon,alt_m,hdop,sats,speed_kmh,course_deg\n");
        xSemaphoreGive(logMutex);
    }
}

/**
 * @brief Đây là hàm "stat-up/init script"
 */
void gpsInit() {
    GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);

#if (GPS_USE_PPS == 1)
    pinMode(GPS_PPS_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(GPS_PPS_PIN), gpsPpsIsr, RISING);
#endif

    printCsvHeader(); // Gọi hàm đã sửa
    __sys_log("[gpsInit] UART2 ready, NMEA @%d.", GPS_BAUD); // Dùng macro log
}

/**
 * @brief Đây là hàm "yourTask"
 */
void TaskGPS(void* pv) {
    __entry("TaskGPS()"); // Tương đương __entry("yourTask()")

    uint32_t lastPrintMs = millis();

    // Buffer cố định, không dùng String
    char csv_line[256];
    char lat_buf[12], lon_buf[12], alt_buf[8], hdop_buf[8];
    char sats_buf[8], spd_buf[8], crs_buf[8];

    for (;;) { // Vòng lặp while(1)
        
        // "your loop get data from sensor"
        while (GPSSerial.available()) {
            gps.encode(GPSSerial.read());
        }

        bool shouldPrint = false;
#if (GPS_USE_PPS == 1)
        if (ppsTick) { 
            ppsTick = false; 
            shouldPrint = true; 
        }
#endif
        if (gps.location.isUpdated() || gps.time.isUpdated() || (millis() - lastPrintMs) > 1000) {
            shouldPrint = true;
        }

        if (shouldPrint) {
            lastPrintMs = millis();
            
            char* p = csv_line;
            lat_buf[0] = '\0'; lon_buf[0] = '\0'; alt_buf[0] = '\0';
            hdop_buf[0] = '\0'; sats_buf[0] = '\0'; spd_buf[0] = '\0'; crs_buf[0] = '\0';

            if (gps.date.isValid() && gps.time.isValid()) {
                p += snprintf(p, sizeof(csv_line) - (p - csv_line), "%d-%02d-%02d,%02d:%02d:%02d,",
                    (int)gps.date.year(), gps.date.month(), gps.date.day(),
                    gps.time.hour(), gps.time.minute(), gps.time.second());
            } else {
                p += snprintf(p, sizeof(csv_line) - (p - csv_line), ",,"); 
            }

            if (gps.location.isValid()) {
                dtostrf(gps.location.lat(), 0, 6, lat_buf); 
                dtostrf(gps.location.lng(), 0, 6, lon_buf);
            }
            p += snprintf(p, sizeof(csv_line) - (p - csv_line), "%s,%s,", lat_buf, lon_buf);

            if (gps.altitude.isValid())   dtostrf(gps.altitude.meters(), 0, 1, alt_buf);
            if (gps.hdop.isValid())       dtostrf(gps.hdop.hdop(), 0, 1, hdop_buf);
            if (gps.satellites.isValid()) snprintf(sats_buf, sizeof(sats_buf), "%lu", gps.satellites.value());
            if (gps.speed.isValid())      dtostrf(gps.speed.kmph(), 0, 2, spd_buf);
            if (gps.course.isValid())     dtostrf(gps.course.deg(), 0, 1, crs_buf);

            p += snprintf(p, sizeof(csv_line) - (p - csv_line), "%s,%s,%s,%s,%s",
                alt_buf, hdop_buf, sats_buf, spd_buf, crs_buf);
            
            // "log to serial"
            if (xSemaphoreTake(logMutex, pdMS_TO_TICKS(100)) == pdTRUE) 
            {
                __coreLog("%s\n", csv_line); 
                xSemaphoreGive(logMutex);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); // Delay (khác 1000ms trong template)
    }
    __exit("TaskGPS()");
    vTaskDelete(NULL); 
}