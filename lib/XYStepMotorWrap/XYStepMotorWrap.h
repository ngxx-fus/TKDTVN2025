#ifndef __XY_STEP_MOTOR_WRAP_H__
#define __XY_STEP_MOTOR_WRAP_H__

#include "Arduino.h"
#include "stdint.h"
#include "../../lib/cArithmeticMacro/cArithmeticMacro.h"
#include "../../lib/cBitWiseMacro/cBitWiseMacro.h"
#include "../../lib/cReturnType/cReturnType.h"
#include "../../lib/cLoopMacro/cLoopMacro.h"

#include <AccelStepper.h>

#if (HM_STEP_MOTOR0_EN == 1)
    // --- 28BYJ-48, FULL4WIRE để tối đa tốc độ + moment ---
    AccelStepper stepper0(AccelStepper::FULL4WIRE,
                          HM_STEP_MOTOR0_PIN0,
                          HM_STEP_MOTOR0_PIN2,
                          HM_STEP_MOTOR0_PIN1,
                          HM_STEP_MOTOR0_PIN3);

    // ============================
    //  API WRAP – TỐI ĐA TỐC ĐỘ
    // ============================
    
    static inline void xStepMotor0Init(float maxSpeed, float acceleration)
    {
        stepper0.setMaxSpeed(maxSpeed);       // gợi ý: 400–500 steps/s
        stepper0.setAcceleration(acceleration);// gợi ý: 50–150 steps/s²
        stepper0.setCurrentPosition(0);
    }

    static inline void xStepMotor0ForwardFast(int32_t nStep)
    {
        stepper0.setSpeed(stepper0.maxSpeed()); // chạy maxSpeed
        int32_t stepsMoved = 0;
        while (stepsMoved < nStep)
        {
            stepper0.runSpeed();  // bỏ gia tốc, tốc độ cố định
            stepsMoved++;
        }
    }

    static inline void xStepMotor0BackFast(int32_t nStep)
    {
        stepper0.setSpeed(-stepper0.maxSpeed());
        int32_t stepsMoved = 0;
        while (stepsMoved < nStep)
        {
            stepper0.runSpeed();
            stepsMoved++;
        }
    }

#endif // HM_STEP_MOTOR0_EN

#if (HM_STEP_MOTOR1_EN == 1)
    // --- 28BYJ-48 motor1, FULL4WIRE ---
    AccelStepper stepper1(AccelStepper::FULL4WIRE,
                          HM_STEP_MOTOR1_PIN0,
                          HM_STEP_MOTOR1_PIN2,
                          HM_STEP_MOTOR1_PIN1,
                          HM_STEP_MOTOR1_PIN3);

    static inline void xStepMotor1Init(float maxSpeed, float acceleration)
    {
        stepper1.setMaxSpeed(maxSpeed);
        stepper1.setAcceleration(acceleration);
        stepper1.setCurrentPosition(0);
    }

    static inline void xStepMotor1ForwardFast(int32_t nStep)
    {
        stepper1.setSpeed(stepper1.maxSpeed());
        int32_t stepsMoved = 0;
        while (stepsMoved < nStep)
        {
            stepper1.runSpeed();
            stepsMoved++;
        }
    }

    static inline void xStepMotor1BackFast(int32_t nStep)
    {
        stepper1.setSpeed(-stepper1.maxSpeed());
        int32_t stepsMoved = 0;
        while (stepsMoved < nStep)
        {
            stepper1.runSpeed();
            stepsMoved++;
        }
    }

#endif // HM_STEP_MOTOR1_EN

#endif // __XY_STEP_MOTOR_WRAP_H__
