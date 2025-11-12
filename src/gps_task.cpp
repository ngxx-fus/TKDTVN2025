/* ===================================================
 * FILE: src/gps_task.cpp (ĐÃ SỬA TẤT CẢ LỖI)
 * ===================================================*/
#include "main.h"       // macro log, logMutex
#include "gps_task.h"
#include <TinyGPSPlus.h>

static HardwareSerial GPSSerial(1); // <<< SỬA 1: Đổi từ 2 sang 1 để tránh xung đột PSRAM
static TinyGPSPlus gps;

#if (GPS_USE_PPS == 1)
static volatile bool ppsTick = false;
void IRAM_ATTR gpsPpsIsr(){ ppsTick = true; }
#endif

// In tiêu đề CSV; nếu không lấy được mutex thì in fallback ra Serial
static void printCsvHeader() {
  // <<< SỬA 2: Sửa lỗi treo (Deadlock) bằng cách in trực tiếp
  // if (logMutex && xSemaphoreTake(logMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
  //  __coreLog("utc_date,utc_time,lat,lon,alt_m,hdop,sats,speed_kmh,course_deg\n");
  //  xSemaphoreGive(logMutex);
  // } else {
  //  Serial.println("utc_date,utc_time,lat,lon,alt_m,hdop,sats,speed_kmh,course_deg");
  // }
  Serial.print("utc_date,utc_time,lat,lon,alt_m,hdop,sats,speed_kmh,course_deg\n");
}

void gpsInit() {
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  #if (GPS_USE_PPS == 1)
    pinMode(GPS_PPS_PIN, INPUT);
    attachInterrupt(digitalPinToInterrupt(GPS_PPS_PIN), gpsPpsIsr, RISING);
  #endif
  printCsvHeader();
  __sys_log("[gpsInit] UART1 ready, NMEA @%d.", GPS_BAUD); // <<< SỬA 3: Cập nhật log từ UART2 -> UART1
}

void TaskGPS(void* pv) {
  __entry("TaskGPS()");
  uint32_t lastPrintMs = millis(), lastDbg = 0;

  char csv_line[256];
  char lat_buf[12], lon_buf[12], alt_buf[8], hdop_buf[8];
  char sats_buf[8], spd_buf[8], crs_buf[8];

  for (;;) {
    while (GPSSerial.available()) gps.encode(GPSSerial.read());

    bool shouldPrint = false;
    #if (GPS_USE_PPS == 1)
      if (ppsTick) { ppsTick = false; shouldPrint = true; }
    #endif
    if (gps.location.isUpdated() || gps.time.isUpdated() || (millis() - lastPrintMs) > 1000) {
      shouldPrint = true;
    }

    if (shouldPrint) {
      lastPrintMs = millis();

      char* p = csv_line;
      lat_buf[0]=lon_buf[0]=alt_buf[0]=hdop_buf[0]=sats_buf[0]=spd_buf[0]=crs_buf[0]='\0';

      if (gps.date.isValid() && gps.time.isValid()) {
        p += snprintf(p, sizeof(csv_line)-(p - csv_line), "%d-%02d-%02d,%02d:%02d:%02d,", // <<< SỬA 4: Fix lỗi biên dịch (thêm dấu cách và gạch dưới)
                      (int)gps.date.year(), gps.date.month(), gps.date.day(),
                      gps.time.hour(), gps.time.minute(), gps.time.second());
      } else {
        p += snprintf(p, sizeof(csv_line)-(p - csv_line), ",,"); // <<< SỬA 4: Fix lỗi biên dịch
      }

      if (gps.location.isValid()) {
        dtostrf(gps.location.lat(), 0, 6, lat_buf);
        dtostrf(gps.location.lng(), 0, 6, lon_buf);
      }
      p += snprintf(p, sizeof(csv_line)-(p - csv_line), "%s,%s,", lat_buf, lon_buf); // <<< SỬA 4: Fix lỗi biên dịch

      if (gps.altitude.isValid())   dtostrf(gps.altitude.meters(), 0, 1, alt_buf);
      if (gps.hdop.isValid())       dtostrf(gps.hdop.hdop(), 0, 1, hdop_buf);
      if (gps.satellites.isValid()) snprintf(sats_buf, sizeof(sats_buf), "%lu", gps.satellites.value());
      if (gps.speed.isValid())      dtostrf(gps.speed.kmph(), 0, 2, spd_buf);
      if (gps.course.isValid())     dtostrf(gps.course.deg(), 0, 1, crs_buf);

      p += snprintf(p, sizeof(csv_line)-(p - csv_line), "%s,%s,%s,%s,%s", // <<< SỬA 4: Fix lỗi biên dịch
                      alt_buf, hdop_buf, sats_buf, spd_buf, crs_buf);

      if (logMutex && xSemaphoreTake(logMutex, pdMS_TO_TICKS(20)) == pdTRUE) {
        __coreLog("%s\n", csv_line);
        xSemaphoreGive(logMutex);
      } else {
        Serial.println(csv_line); // fallback
      }
    }

    // debug nhẹ mỗi 1s để biết parser đang chạy
    if (millis() - lastDbg > 1000) {
      lastDbg = millis();
      __sys_log("[GPS] chars=%lu, valid=%d, sats=%lu, hdop=%.1f",
        (unsigned long)gps.charsProcessed(),
        gps.location.isValid(),
        gps.satellites.isValid()? gps.satellites.value() : 0,
        gps.hdop.isValid()? gps.hdop.hdop() : -1.0f
      );
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }

  __exit("TaskGPS()");
  vTaskDelete(NULL);
}