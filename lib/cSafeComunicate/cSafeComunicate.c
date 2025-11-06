#include "cSafeComunicate.h"

/// TRANSMIT SECTION //////////////////////////////////////////////////////////////////////////////

int stComputeEncodedDataSize(int rawDataSizeInByte) {
    int bitNum = rawDataSizeInByte * 8;
    int frameBytes = (bitNum + 6) / 7; // This is ceil(bitNum / 7)
    return frameBytes;
}

int stPack7(const uint8_t* raw, int size, uint8_t *encoded, uint8_t dc)
{
    int totalBits = size * 8;
    int outIndex = 0;
    int bitIndex = 0;

    // Use the D/C bit (0 or 1) shifted to the MSB position
    uint8_t dc_bit = (dc & 1) << 7;

    while (bitIndex < totalBits)
    {
        uint8_t data7 = 0;

        for (int i = 0; i < 7; i++)
        {
            data7 <<= 1; // Shift accumulator left

            if (bitIndex < totalBits)
            {
                int bytePos = bitIndex / 8;
                int bitPos  = 7 - (bitIndex % 8); // Get bit from MSB to LSB
                uint8_t bit = (raw[bytePos] >> bitPos) & 1;

                data7 |= bit; // Add the bit to our 7-bit payload
                bitIndex++;
            }
            // If bitIndex >= totalBits, we just keep shifting, padding with 0
        }

        // Combine the D/C flag with the 7-bit payload
        encoded[outIndex++] = dc_bit | data7; // data7 is already max 7 bits
    }

    return outIndex; // Returns the number of bytes written to 'encoded'
}

void stEncodeCommandArr(uint8_t* raw, int size, uint8_t * encoded) {
    stPack7(raw, size, encoded, 0); // 0 for Command
}

void stEncodeDataArr(uint8_t* raw, int size, uint8_t * encoded) {
    stPack7(raw, size, encoded, 1); // 1 for Data
}


/// RECEIVE SECTION //////////////////////////////////////////////////////////////////////////////

void stDecoderInit(stDecoderState* state) {
    state->data_stream.accumulator = 0;
    state->data_stream.bit_count = 0;
    state->cmd_stream.accumulator = 0;
    state->cmd_stream.bit_count = 0;
}

void stDecodeChunk(stDecoderState* state,
                                 const uint8_t* encoded, int encoded_size,
                                 uint8_t* rawData, int* dataWritten,
                                 uint8_t* rawCmd, int* cmdWritten)
{
    // Reset write counts for this chunk
    *dataWritten = 0;
    *cmdWritten = 0;

    for (int i = 0; i < encoded_size; i++) {
        uint8_t dc_bit = (encoded[i] >> 7) & 1;
        uint8_t payload7 = encoded[i] & 0x7F;

        stBitstreamState* stream;
        uint8_t* outBuf;
        int* outWritten;

        // Select the correct stream (Data or Command)
        if (dc_bit == 1) {
            stream = &state->data_stream;
            outBuf = rawData;
            outWritten = dataWritten;
        } else {
            stream = &state->cmd_stream;
            outBuf = rawCmd;
            outWritten = cmdWritten;
        }

        // Add the 7-bit payload to that stream's accumulator
        stream->accumulator = (stream->accumulator << 7) | payload7;
        stream->bit_count += 7;

        // While there are enough bits to form an 8-bit byte...
        while (stream->bit_count >= 8) {
            // Calculate the bit position of the 8-bit byte
            int shift = stream->bit_count - 8;

            // Extract the top 8 bits
            uint8_t outByte = (stream->accumulator >> shift) & 0xFF;

            // Write the byte to the correct output buffer (at its next index)
            outBuf[*outWritten] = outByte;
            (*outWritten)++; // Increment the write count

            // Update the bit count
            stream->bit_count -= 8;

            // Mask the accumulator to remove the bits we just used
            // (1 << 0) - 1 = 0
            // (1 << 5) - 1 = 0b11111
            if (stream->bit_count > 0) {
                stream->accumulator &= (1 << stream->bit_count) - 1;
            } else {
                stream->accumulator = 0;
            }
        }
    }
}