#include "espSerialWrap.h"

#if (SYS_SEMAPHORE_LOG_EN == 1)
    SemaphoreHandle_t logMutex = NULL;
#endif