#include "atgm336hWrap.h"

#if (SENSOR_ATGM336H_EN == 1)

/* --- Global Variable Instantiation --- */
HardwareSerial       gpsSerial(GPS_SERIAL_NUM);
TinyGPSPlus          gpsDevice;
atgm336hData_t       gpsData;
std::atomic<uint8_t> gpsDataUpdateFlag(0); // Initialize with 0

#endif /// (SENSOR_ATGM336H_EN == 1)