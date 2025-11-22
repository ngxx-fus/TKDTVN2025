#ifndef __CSAFE_COMMUNICATE_H__
#define __CSAFE_COMMUNICATE_H__

/// EXTEND FOR C++ //////////////////////////////////////////////////
#ifdef __cplusplus                                                ///
extern "C" {                                                      ///
#endif                                                            ///
/////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <string.h>

/// @brief Status definitions
#define STATUS_OKE  0
#define STATUS_ERR -1

/// @brief Data type definition as requested
typedef int def;

/// @brief Maximum buffer size definition (Adjust as needed)
#ifndef CPACKAGING_SIZE
#define CPACKAGING_SIZE 1024
#endif

/// @brief CRC Polynomial
#define CRC_POLYNOMIAL 0x1021

/// @brief Calculate CRC16 (Poly 0x1021) supporting both HW (ESP32) and SW
/// @param ByteArr Pointer to data buffer
/// @param ByteArrSize Size of data buffer
/// @param CRCInitValue Initial CRC value (usually 0 or 0xFFFF)
/// @return Calculated CRC16 value
uint16_t computeCRC16(void* ByteArr, int ByteArrSize, int16_t CRCInitValue);

/// @brief Encode data into the specific frame format
/// @param ID Frame ID (1 byte)
/// @param ByteArr Pointer to raw data payload
/// @param ByteArrSize Size of raw data payload
/// @param PackedByteArr Output buffer for the complete frame
/// @param PackedByteArrMaxSize Maximum size of the output buffer
/// @return Number of bytes packed (positive) or STATUS_ERR
def DataFrameEncode(uint8_t ID, void* ByteArr, int ByteArrSize, void* PackedByteArr, int PackedByteArrMaxSize);

/// @brief Decode and validate a raw frame
/// @param PackedByteArr Pointer to the complete raw frame
/// @param PackedByteArrSize Size of the raw frame
/// @param ID Pointer to store the extracted ID
/// @param ByteArr Output buffer for extracted data
/// @param ByteArrMaxSize Maximum size of the output buffer
/// @return Number of data bytes extracted (positive) or STATUS_ERR if CRC/Format fails
def DataFrameDecode(void* PackedByteArr, int PackedByteArrSize, uint8_t *ID, void* ByteArr, int ByteArrMaxSize);

/// @brief Poll individual bytes to detect and validate a frame using FSM
/// @param NewByte The incoming byte from UART/Stream
/// @return STATUS_OKE if a valid frame is fully received and verified in the internal buffer, otherwise STATUS_ERR
def DataFramePoll(uint8_t NewByte);

/// EXTEND FOR C++ //////////////////////////////////////////////////
#ifdef __cplusplus                                                ///
}                                                                 ///
#endif                                                            ///
/////////////////////////////////////////////////////////////////////

#endif // __CSAFE_COMMUNICATE_H__