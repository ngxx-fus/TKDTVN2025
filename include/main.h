#include "Arduino.h"
#include "stdio.h"
#include "stdarg.h"
#include "stdint.h"
#include "../lib/cArithmeticMacro/cArithmeticMacro.h"
#include "../lib/arduinoNanoSerialWarp/SerialWarp.h"
#include "../lib/cBitWiseMacro/cBitWiseMacro.h"
#include "../lib/cReturnType/cReturnType.h"
#include "../lib/cLoopMacro/cLoopMacro.h"

#if (LIGHT_TICK_EN == 1)

    static void lightTickInit(){
        __entry("lightTickInit()");
        pinMode(LIGHT_TICK_PIN, OUTPUT);
        __exit("lightTickInit()");
    }

    static void lightTickRun() {
        static unsigned long lastEntry = 0;
        unsigned long now = millis();
        unsigned long elapsed = now - lastEntry;

        digitalWrite(LIGHT_TICK_PIN, HIGH);
        delay(LIGHT_TICK_TIME_ON);
        digitalWrite(LIGHT_TICK_PIN, LOW);

        if (elapsed < (LIGHT_TICK_TIME_ON + LIGHT_TICK_TIME_OFF)) {
            delay((LIGHT_TICK_TIME_ON + LIGHT_TICK_TIME_OFF) - elapsed);
        }

        lastEntry = millis();
    }


#endif

#if (HM_XY_STEP_MOTOR0_EN == 1) || (HM_XY_STEP_MOTOR1_EN == 1)
    #include "../lib/XYStepMotorWrap/XYStepMotorWrap.h"
    
#endif