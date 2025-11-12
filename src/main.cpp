#include "main.h"

void setup(){
    Serial.begin(115200);
    __entry("setup()");
    lightTickInit();
    xyStepMotorInit();

    __exit("setup()");
}

void loop(){
    lightTickRun();
    /// forward
    // REP(j, 0, 100)
    // REV(i, 90000, 0){
    //     #if (HM_XY_STEP_MOTOR0_EN == 1)
    //         xStepMotor0ForwardFast(1);
    //     #endif
    //     #if (HM_XY_STEP_MOTOR1_EN == 1)
    //         xStepMotor1ForwardFast(1);
    //     #endif
    // }
    REP(j, 0, 100)
    REV(i, 90000, 0){
        #if (HM_XY_STEP_MOTOR0_EN == 1)
            xStepMotor0BackFast(1);
        #endif
        #if (HM_XY_STEP_MOTOR1_EN == 1)
            xStepMotor1BackFast(1);
        #endif
    }
    __sys_log("[loop] running...");

}