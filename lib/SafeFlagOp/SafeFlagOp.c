#include "SafeFlagOp.h"

/* Mask helper */
static inline uint32_t bitMask(int bit) {
    if (bit < 0 || bit > 31) return 0;
    return (1u << bit);
}

/* Atomic load */
static inline uint32_t atomic_load_u32(const uint32_t *ptr) {
    uint32_t val;
    __atomic_load(ptr, &val, __ATOMIC_RELAXED);
    return val;
}

/* ============================================================
   Check
   ============================================================ */
int __hasFlagBit(const flag_t *f, int bit) {
    uint32_t mask = bitMask(bit);
    if (!mask) return 0;

    return (atomic_load_u32(&f->v) & mask) != 0;
}

int __hasFlagBits(const flag_t *f, int bit, ...) {
    va_list args;
    va_start(args, bit);

    uint32_t v = atomic_load_u32(&f->v);
    int ok = 1;

    int b = bit;
    while (b >= 0) {
        uint32_t m = bitMask(b);
        if (!m) {
            ok = 0;
            break;
        }
        if ((v & m) == 0) {
            ok = 0;
            break;
        }

        b = va_arg(args, int);
    }

    va_end(args);
    return ok;
}

/* ============================================================
   Set
   ============================================================ */
void __setFlagBit(flag_t *f, int bit) {
    uint32_t mask = bitMask(bit);
    if (!mask) return;

    __atomic_fetch_or(&f->v, mask, __ATOMIC_RELAXED);
}

void __setFlagBits(flag_t *f, int bit, ...) {
    va_list args;
    va_start(args, bit);

    uint32_t mask = 0;

    int b = bit;
    while (b >= 0) {
        uint32_t m = bitMask(b);
        if (m) mask |= m;

        b = va_arg(args, int);
    }

    va_end(args);

    if (mask)
        __atomic_fetch_or(&f->v, mask, __ATOMIC_RELAXED);
}

/* ============================================================
   Clear
   ============================================================ */
void __clearFlagBit(flag_t *f, int bit) {
    uint32_t mask = bitMask(bit);
    if (!mask) return;

    __atomic_fetch_and(&f->v, ~mask, __ATOMIC_RELAXED);
}

void __clearFlagBits(flag_t *f, int bit, ...) {
    va_list args;
    va_start(args, bit);

    uint32_t mask = 0;

    int b = bit;
    while (b >= 0) {
        uint32_t m = bitMask(b);
        if (m) mask |= m;

        b = va_arg(args, int);
    }

    va_end(args);

    if (mask)
        __atomic_fetch_and(&f->v, ~mask, __ATOMIC_RELAXED);
}
