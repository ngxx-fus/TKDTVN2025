#ifndef __SAFE_FLAG_OP_H__
#define __SAFE_FLAG_OP_H__

#include <stdint.h>
#include <stdarg.h>

typedef union {
    struct{
        uint32_t b0 : 1;
        uint32_t b1 : 1;
        uint32_t b2 : 1;
        uint32_t b3 : 1;
        uint32_t b4 : 1;
        uint32_t b5 : 1;
        uint32_t b6 : 1;
        uint32_t b7 : 1;
        uint32_t b8 : 1;
        uint32_t b9 : 1;
        uint32_t b10 : 1;
        uint32_t b11 : 1;
        uint32_t b12 : 1;
        uint32_t b13 : 1;
        uint32_t b14 : 1;
        uint32_t b15 : 1;
        uint32_t b16 : 1;
        uint32_t b17 : 1;
        uint32_t b18 : 1;
        uint32_t b19 : 1;
        uint32_t b20 : 1;
        uint32_t b21 : 1;
        uint32_t b22 : 1;
        uint32_t b23 : 1;
        uint32_t b24 : 1;
        uint32_t b25 : 1;
        uint32_t b26 : 1;
        uint32_t b27 : 1;
        uint32_t b28 : 1;
        uint32_t b29 : 1;
        uint32_t b30 : 1;
        uint32_t b31 : 1;
    };
    uint32_t v;
} flag_t;

/* Check */
int __hasFlagBit(const flag_t *f, int bit);
int __hasFlagBits(const flag_t *f, int bit, ...);

/* Set */
void __setFlagBit(flag_t *f, int bit);
void __setFlagBits(flag_t *f, int bit, ...);

/* Clear */
void __clearFlagBit(flag_t *f, int bit);
void __clearFlagBits(flag_t *f, int bit, ...);

#endif
