#include "../include/localHelperAndUtil.h"

void Task01(void* pv){
    __entry("Task01()");
    /// Task01 : Blink LED with period 200 tick
    while(1){
        GPIO.out_w1tc = __mask32(HM_LED0_PIN);
        vTaskDelay(100);
        GPIO.out_w1ts = __mask32(HM_LED0_PIN);
        vTaskDelay(100);
    }

    __exit("Task01()");
}

void Task02(void* pv){
    __entry("Task02()");
    /// Task02 : Blink LED with period 700 tick
    while(1){
        GPIO.out_w1tc = __mask32(HM_LED1_PIN);
        vTaskDelay(350);
        GPIO.out_w1ts = __mask32(HM_LED1_PIN);
        vTaskDelay(350);
    }
    __exit("Task02()");
}

void Task03(void *pv){
    __entry("Task03()");

    MPU6050 mpu;

    // --- Init I2C ---
    Wire.begin();
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // --- Init MPU6050 ---
    mpu.initialize();
    if (!mpu.testConnection()) {
        __log("[Task03] "  "MPU6050 connection failed!");
        vTaskDelete(NULL); // Stop this task if the sensor didn't respond
    } else {
        __log("[Task03] "  "MPU6050 connected successfully.");
    }

    // Data variables
    int16_t ax, ay, az;
    int16_t gx, gy, gz;

    // Main loop
    while (1) {
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

        // Print onto Serial / Log
        __log("[Task03] " "A[x:%d y:%d z:%d] G[x:%d y:%d z:%d]", ax, ay, az, gx, gy, gz);

        // Delay 750 ms
        vTaskDelay(pdMS_TO_TICKS(350));
    }

    __exit("Task03()");
    vTaskDelete(NULL);
}

void setup(){
    /// For serial log | Baudrate: 115200
    Serial.begin(115200);
    /// For semaphore log
    semaphoreLogInit();
    /// Other set-up
    __entry("setup()");

    /// Set-up LED for Task01 and Task02
    gpio_config_t outPin = {
        .pin_bit_mask = __masks64(HM_LED0_PIN, HM_LED1_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&outPin);

    /// Add Task01 and Task02 and Task03
    __log("[+] Add Task01");
    xTaskCreate(Task01, "Task01", 2048, NULL, 1, NULL);
    __log("[+] Add Task02");
    xTaskCreate(Task02, "Task02", 2048, NULL, 1, NULL);
    __log("[+] Add Task03");
    xTaskCreate(Task03, "Task03", 2048, NULL, 1, NULL);

    __exit("setup()");
}

void loop(){
    /// Infinity lock :>
    __log("[loop] Put loop() to infinity sleep!");
    vTaskDelay(portMAX_DELAY);
}
