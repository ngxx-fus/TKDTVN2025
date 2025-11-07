#ifndef __SAFE_COMMUNICATE_H__
#define __SAFE_COMMUNICATE_H__

/// EXTEND FOR C++ //////////////////////////////////////////////////
#ifdef __cplusplus                                                ///
extern "C" {                                                      ///
#endif                                                            ///
/////////////////////////////////////////////////////////////////////

#include "stdint.h"

/* Dataframe:
        MSB 7 6 5 4 3 2 1 0 LSB
        [D/C][    DATA    ]
    0 : Command
    1 : Data
*/

/// TRANSMIT SECTION //////////////////////////////////////////////////////////////////////////////

/**
 * @brief Computes the required size of the output buffer for encoding.
 * * @param rawDataSizeInByte The number of raw 8-bit bytes to be encoded.
 * @return The number of 7-bit-payload bytes that will be generated.
 */
int stComputeEncodedDataSize(int rawDataSizeInByte);

/**
 * @brief Core packing function. Packs 8-bit data into 7-bit payloads.
 * (Internal use)
 */
int stPack7(const uint8_t* raw, int size, uint8_t *encoded, uint8_t dc);

/**
 * @brief Encodes an array of 8-bit data as "Command" frames.
 * * @param raw The input buffer of 8-bit data.
 * @param size The number of bytes in the raw buffer.
 * @param encoded The output buffer. Must be large enough (see stComputeEncodedDataSize).
 */
void stEncodeCommandArr(uint8_t* raw, int size, uint8_t * encoded);

/**
 * @brief Encodes an array of 8-bit data as "Data" frames.
 * * @param raw The input buffer of 8-bit data.
 * @param size The number of bytes in the raw buffer.
 * @param encoded The output buffer. Must be large enough (see stComputeEncodedDataSize).
 */
void stEncodeDataArr(uint8_t* raw, int size, uint8_t * encoded);


/// RECEIVE SECTION //////////////////////////////////////////////////////////////////////////////

/**
 * @brief Internal state for a single bitstream (data or command).
 */
typedef struct {
    uint16_t accumulator; // Can hold up to 14 bits (from two 7-bit payloads)
    int bit_count;
} stBitstreamState;

/**
 * @brief Holds the complete state for the decoder, managing both streams.
 */
typedef struct {
    stBitstreamState data_stream;
    stBitstreamState cmd_stream;
} stDecoderState;

/**
 * @brief Initializes a new decoder state.
 * Call this once before first use.
 */
void stDecoderInit(stDecoderState* state);

/**
 * @brief Decodes a chunk of incoming encoded bytes and sorts them.
 * This function is stateful and must be called with the same 'state'
 * object every time you receive new data.
 *
 * @param state         A pointer to your persistent stDecoderState object.
 * @param encoded       The input buffer of new encoded bytes.
 * @param encoded_size  The number of bytes in the encoded buffer.
 * @param rawData       The output buffer for 8-bit Data.
 * @param dataWritten   (Output) The number of 8-bit bytes written to rawData.
 * @param rawCmd        The output buffer for 8-bit Commands.
 * @param cmdWritten    (Output) The number of 8-bit bytes written to rawCmd.
 */
void stDecodeChunk(stDecoderState* state, const uint8_t* encoded, int encoded_size, uint8_t* rawData, int* dataWritten, uint8_t* rawCmd, int* cmdWritten);


/// EXTEND FOR C++ //////////////////////////////////////////////////
#ifdef __cplusplus                                                ///
}                                                                 ///
#endif                                                            ///
/////////////////////////////////////////////////////////////////////
#endif // __SAFE_COMMUNICATE_H__