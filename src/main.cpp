/* ===================================================
 * FILE: src/main.cpp
 * ===================================================*/

#include "main.h"          // macro log, khai báo chung
#include <cReturnType.h>
#include <Wire.h>          // I2C

// (Tùy chọn) Hàm quét I2C một lần để debug MPU6050 khi cần
static void i2cScanOnce() {
  __sys_log("[I2C] Scan begin");
  uint8_t found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) { __sys_log("[I2C] Found 0x%02X", addr); found++; }
  }
  __sys_log("[I2C] Scan end. Found %u device(s)", found);
}

#if (LIGHT_TICK_EN == 1)
void TaskLightTick(void* pv){
  __entry("TaskLightTick()");
  gpio_config_t outPin = {
    .pin_bit_mask = __mask64(LIGHT_TICK_PIN),
    .mode = GPIO_MODE_OUTPUT,
    .pull_up_en = GPIO_PULLUP_DISABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&outPin);
  __sys_log("[TaskLightTick] Info: GPIO=%d, TimeOn=%d TimeOff=%d",
            LIGHT_TICK_PIN, LIGHT_TICK_TIME_ON, LIGHT_TICK_TIME_OFF);
  while(1){
    GPIO.out_w1tc = __mask32(LIGHT_TICK_PIN);
    vTaskDelay(LIGHT_TICK_TIME_ON);
    GPIO.out_w1ts = __mask32(LIGHT_TICK_PIN);
    vTaskDelay(LIGHT_TICK_TIME_OFF);
  }
  __exit("TaskLightTick()");
}
#endif

#if (SENSOR_MPU6050_EN == 1)
void TaskMPU6050(void *pv){
  __entry("TaskMPU6050()");
  if (mpu6050Init() != STATUS_OKE) {
    __sys_log("[TaskMPU6050] MPU6050 connection failed!");
    vTaskDelete(NULL);
  } else {
    __sys_log("[TaskMPU6050] MPU6050 connected successfully.");
  }
  while (1) {
    mpu6050Measure();
    __sys_log("[TaskMPU6050] a[x:%d,y:%d,z:%d] g[x:%d,y:%d,z:%d]",
              mpuData.ax, mpuData.ay, mpuData.az, mpuData.gx, mpuData.gy, mpuData.gz);
    vTaskDelay(pdMS_TO_TICKS(350));
  }
  __exit("TaskMPU6050()");
  vTaskDelete(NULL);
}
#endif

#if (FIREBASE_SYNC_EN == 1)
void TaskFirebaseSync(void* pv){
  __entry("TaskFirebaseSync()");
  while(1){
    if(fbUploadMPU6050Data()!=STATUS_OKE) __sys_log("[Task04] Upload failed!");
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
  __exit("TaskFirebaseSync()");
}
#endif

#if (SENSOR_HCSR04_EN == 1)
void TaskHCSR04(void *pv){
  DEFAULT_RETURN_STATUS rv = STATUS_OKE;
  hcsr04Dev.T0 = hcsr04Dev.T1 = hcsr04Dev.T2 = hcsr04Dev.T3 = HCSR04_C_PIN;
  hcsr04Dev.E0 = HCSR04_0_PIN; hcsr04Dev.E2 = HCSR04_2_PIN;
  hcsr04Dev.E1 = HCSR04_1_PIN; hcsr04Dev.E3 = HCSR04_3_PIN;
  hcsr04Init();
  while(1){
    rv = hcsr04MeasureAll();
    if (rv != STATUS_OKE){
      __sys_err("[TaskHCSR04] hcsr04MeasureAll(): %s", DEFAULT_RETURN_STATUS_STR(rv));
    }
    __sys_log("[TaskHCSR04] F:%d R:%d B:%d L:%d",
              hcsr04Data.arr[0], hcsr04Data.arr[1], hcsr04Data.arr[2], hcsr04Data.arr[3]);
    vTaskDelay(pdMS_TO_TICKS(950));
  }
}
#endif

void setup(){
  Serial.begin(115200);
  semaphoreLogInit();
  __entry("setup()");

  // I2C cho các cảm biến dùng Wire (MPU6050,...)
  Wire.begin(21, 22, 100000);
  Wire.setTimeOut(50);
  // i2cScanOnce(); // bật khi cần debug I2C

  #if (LIGHT_TICK_EN == 1)
    __sys_log("[setup] [+] Add TaskLightTick");
    // xTaskCreate(TaskLightTick, "TaskLightTick", 2048, NULL, 1, NULL);
  #endif

  #if (SENSOR_MPU6050_EN == 1)
    __sys_log("[setup] [+] Add TaskMPU6050");
    xTaskCreate(TaskMPU6050, "TaskMPU6050", 2048, NULL, 1, NULL);
  #endif

  #if (SENSOR_HCSR04_EN == 1)
    __sys_log("[setup] [+] Add TaskHCSR04");
    // xTaskCreate(TaskHCSR04, "TaskHCSR04", 2048, NULL, 1, NULL);
  #endif

  #if (FIREBASE_SYNC_EN == 1)
    wfInit(); fbInit();
    __sys_log("[setup] [+] Add TaskFirebaseSync");
    xTaskCreate(TaskFirebaseSync, "TaskFirebaseSync", 2048, NULL, 1, NULL);
  #endif

  #if (SENSOR_GPS_EN == 1)
    __sys_log("[setup] ===== TEST CODE MOI: DANG DUNG PIN 32/33 ====="); // <--- THÊM DÒNG NÀY
    __sys_log("[setup] [+] Initializing GPS...");
    gpsInit();
    __sys_log("[setup] [+] Add TaskGPS");
    xTaskCreate(TaskGPS, "TaskGPS", 4096, NULL, 1, NULL);
  #endif

  __exit("setup()");
}

void loop(){
  __sys_log("[loop] Put loop() to infinity sleep!");
  vTaskDelay(portMAX_DELAY);
}
