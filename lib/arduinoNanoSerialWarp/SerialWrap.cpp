#include "SerialWarp.h"

void __coreLog(const char* fmt, ...) {
    static char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    Serial.print(buf);
    Serial.print('\r');
}