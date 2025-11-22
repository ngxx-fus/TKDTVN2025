/// @file   projectConfig.h
/// @author Nguyen Thanh Phu
/// @brief  Provides configuration macros for components, setup, and logging.

#ifndef __PROJECT_CONFIG_H__
#define __PROJECT_CONFIG_H__

#include "stdlib.h"
#include "stdarg.h"
#include "stdint.h"

/// CONFIG | LOG //////////////////////////////////////////////////////////////////////////////////

#define SYS_LOG_L0_EN           1
#define SYS_LOG_L1_EN           0
#define SYS_LOG_L2_EN           0
#define SYS_LOG_ERR_EN          1
#define SYS_LOG_L0_ENTRY        1
#define SYS_LOG_L0_EXIT         1
#define SYS_LOG_L1_ENTRY        0
#define SYS_LOG_L1_EXIT         0
#define SYS_LOG_L2_ENTRY        0
#define SYS_LOG_L2_EXIT         0

/// CONFIG | COMPONENTS ///////////////////////////////////////////////////////////////////////////

#define LIGHT_TICK_EN           1
#define FIREBASE_SYNC_EN        1
#define SENSOR_HCSR04_EN        0
#define SENSOR_MPU6050_EN       1
#define SENSOR_ATGM336H_EN      1
#define LAN_DATA_EXCHANGE_EN    1

/// CONFIG | COMPONENTS | DETAILS /////////////////////////////////////////////////////////////////

#if (LIGHT_TICK_EN == 1)
    #define LIGHT_TICK_PIN          19
    #define LIGHT_TICK_TIME_ON      100
    #define LIGHT_TICK_TIME_OFF     50
    #define LIGHT_TICK_CYCLIC_PATTERN "10100100000000000000000"
#endif /// (LIGHT_TICK_EN == 1)

#if (SENSOR_MPU6050_EN == 1)
    #define MPU6050_SCL 22
    #define MPU6050_SDA 21
    #define MPU6050_MEASURE_INTERVAL 350000          /// micro-sec
#endif

#if (SENSOR_HCSR04_EN == 1)
    #define HCSR04_C_PIN            16              /// Comon Trigger
    #define HCSR04_0_PIN            19              /// Front
    #define HCSR04_1_PIN            5               /// Right
    #define HCSR04_2_PIN            18              /// Back
    #define HCSR04_3_PIN            17              /// Left
    #define HCSR04_WAIT_FOR_HIGH    25000           /// Wait for echo HIGH after trigger
    #define HCSR04_WAIT_FOR_LOW     30000           /// Wait for echo LOW after posedge
    #define HCSR04_WAIT_BEFORE_CONT 60              /// Wait before next trigger
    #define HCSR04_MEASURE_INTERVAL 950000
#endif /// (SENSOR_HCSR04_EN == 1)

#if (SENSOR_ATGM336H_EN == 1)
    #define SENSOR_GPS_EN           1
    #define GPS_RX_PIN              33  /// (TX ON ATGM336H)
    #define GPS_TX_PIN              32  /// (RX ON ATGM336H)
    #define GPS_BAUD                9600
    #define GPS_PPS_EN              0
    #define GPS_PPS_PIN             35
    #define GPS_SERIAL_NUM          2
    #define GPS_MEASURE_INTERVAL    1000000
#endif  /// (SENSOR_ATGM336H_EN == 1)

#if (LAN_DATA_EXCHANGE_EN == 1)

    #define ELDE_FRAME_BEGIN            "FRAME_BEGIN"
    #define ELDE_FRAME_END              "FRAME_END"
    #define ELDE_TAG_CRC                "CRC"


    #define ELDE_RX_BUF_SIZE        512

    #define CRC_POLYNOMIAL          0x1021
    #define ESP_USE_CRC_HW          0

    #define ID_SENSOR_BASE          40
    #define ID_SENSOR_MPU6050       (ID_SENSOR_BASE + 1)
    #define ID_SENSOR_HCSR04        (ID_SENSOR_BASE + 2)
    #define ID_SENSOR_ATGM336H      (ID_SENSOR_BASE + 3)

    #define ID_STATUS_BASE          200
    #define ID_STATUS_OKE           (ID_STATUS_BASE)
    #define ID_STATUS_ERR           (ID_STATUS_BASE + 1)

    #define THIS_ESP_UDP_PORT       2578
    #define THIS_ESP_TCP_PORT       8579

    #define SERVER_UDP_PORT         3546
    #define SERVER_TCP_PORT         9527

    /**
     * General frame:   <"FRAME_BEGIN"> <BinaryByte-ID> <N BinaryByte-Data> <"CRC"> <CRCByteHigh><CRCByteLow> <"FRAME_END">
     * Note: 
     *      -   CRC16 only include payload field! 
     *      -   If data is corrupted ---> marked as not-received (nothin' happend!)
     *      -   "FRAME_BEGIN"   = 4652414d455f424547494e
     *      -   "FRAME_END"     = 4652414d455f454e44
     *      -   "CRC"           = 435243
     */

#endif /// (LAN_DATA_EXCHANGE_EN == 1)

#if (FIREBASE_SYNC_EN == 1)
    #define FBRTDB_URL              "https://tkdtvn2025-default-rtdb.asia-southeast1.firebasedatabase.app/"
    #define FB_API_KEY              "AIzaSyBC-o4n9PbYDeWlKkxo-Jogn7EhwA96MZ8"
    #define FB_USER_EMAIL           "huu-nam-3204@tkdtvn2025.iam.gserviceaccount.com"/// "nthanhphu.k22.hcmute@gmail.com"
    #define FB_USER_PASSWORD        "@asjd2361qeuFASl:kHJDASsn#wer"
    #define FBRTDB_ROOT_PATH        "/"
    #define FBRTDB_MPU6050_PATH     FBRTDB_ROOT_PATH "mpu6050/"
    #define FBRTDB_HCSR04_PATH      FBRTDB_ROOT_PATH "hcrs04/"
    #define FBRTDB_ATGM336H_PATH    FBRTDB_ROOT_PATH "atgm336h/"

    #define FB_SYNC_DELAY           5000000 /// In micro-sec
#endif /// (FIREBASE_SYNC_EN == 1)

#if (LAN_DATA_EXCHANGE_EN == 1) ||  (FIREBASE_SYNC_EN == 1)
    #define WIFI_SSID               "VEDC2025_LUX_TEAM"
    #define WIFI_PASSWORD           "VEDC2025_LUX_TEAM"
#endif /// (LAN_DATA_EXCHANGE_EN == 1) ||  (FIREBASE_SYNC_EN == 1)

#endif  /// __PROJECT_CONFIG_H__