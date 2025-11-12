#ifndef __XY_STEP_MOTOR_WRAP_H__
#define __XY_STEP_MOTOR_WRAP_H__

// Keep <utility> for std::pair and <vector> for std::vector
#include <utility>
#include <vector>

#include "Arduino.h"
#include "stdint.h"
#include "../../lib/cArithmeticMacro/cArithmeticMacro.h"
#include "../../lib/cBitWiseMacro/cBitWiseMacro.h"
#include "../../lib/cReturnType/cReturnType.h"
#include "../../lib/cLoopMacro/cLoopMacro.h"

#include <AccelStepper.h>

/// xysm = XY-Step-Motor

// PIN DEFINITIONS

#ifndef HM_XY_STEP_MOTOR0_PIN0
    #define HM_XY_STEP_MOTOR0_PIN0     -1
#endif
#ifndef HM_XY_STEP_MOTOR0_PIN1
    #define HM_XY_STEP_MOTOR0_PIN1     -1
#endif
#ifndef HM_XY_STEP_MOTOR0_PIN2
    #define HM_XY_STEP_MOTOR0_PIN2     -1
#endif
#ifndef HM_XY_STEP_MOTOR0_PIN3 
    #define HM_XY_STEP_MOTOR0_PIN3     -1
#endif
#ifndef HM_XY_STEP_MOTOR1_PIN0
    #define HM_XY_STEP_MOTOR1_PIN0     -1
#endif
#ifndef HM_XY_STEP_MOTOR1_PIN1
    #define HM_XY_STEP_MOTOR1_PIN1     -1
#endif
#ifndef HM_XY_STEP_MOTOR1_PIN2
    #define HM_XY_STEP_MOTOR1_PIN2     -1
#endif
#ifndef HM_XY_STEP_MOTOR1_PIN3
    #define HM_XY_STEP_MOTOR1_PIN3     -1
#endif



static float xysmMaxSpeed = 400.0f; 
static float xysmMaxAcceleration = 100.0f; 

// Static definition, suitable for inclusion directly in .ino
static AccelStepper stepper0(AccelStepper::FULL4WIRE,
                            HM_XY_STEP_MOTOR0_PIN0,
                            HM_XY_STEP_MOTOR0_PIN2,
                            HM_XY_STEP_MOTOR0_PIN1,
                            HM_XY_STEP_MOTOR0_PIN3);
static AccelStepper stepper1(AccelStepper::FULL4WIRE,
                            HM_XY_STEP_MOTOR1_PIN0,
                            HM_XY_STEP_MOTOR1_PIN2,
                            HM_XY_STEP_MOTOR1_PIN1,
                            HM_XY_STEP_MOTOR1_PIN3);

// Use 'long' to match AccelStepper's internal type
typedef std::pair<long, long> xysmPoint_t;

// Vector storing target (X, Y) coordinates
static std::vector<xysmPoint_t> xysmTargetPoints;
// Vector storing wait time (in microseconds) at each point
static std::vector<unsigned long> xysmWaitTimes_us;


static size_t        xysmCurrentPointIndex = 0;
static bool          xysmIsWaiting = false;
static unsigned long xysmWaitStartTime_us = 0;
// Flag to activate the sequence following
static bool          xysmSequenceActive = false;




/// Initializes both motors with max speed and acceleration.
inline void xysmInit(float maxSpeed, float acceleration) {
    xysmMaxSpeed = maxSpeed;
    xysmMaxAcceleration = acceleration;

    stepper0.setMaxSpeed(xysmMaxSpeed);
    stepper0.setAcceleration(xysmMaxAcceleration);
    stepper0.setCurrentPosition(0);

    stepper1.setMaxSpeed(xysmMaxSpeed);
    stepper1.setAcceleration(xysmMaxAcceleration);
    stepper1.setCurrentPosition(0);
}

/// Manually moves the motors by a relative distance (disables sequence).
inline void xysmMove(long dx, long dy) {
    // Disable sequence on manual override
    xysmSequenceActive = false; 
    stepper0.move(dx);
    stepper1.move(dy);
}

/// Adds an absolute (X, Y) point to the sequence with a specific wait time.
inline void xysmAddTargetPoint(long x, long y, unsigned long wait_us) {
    xysmTargetPoints.push_back(std::make_pair(x, y));
    xysmWaitTimes_us.push_back(wait_us);
}

/// Clears all target points from the sequence.
inline void xysmClearTargetPoints() {
    xysmTargetPoints.clear();
    xysmWaitTimes_us.clear();
    xysmCurrentPointIndex = 0;
    xysmSequenceActive = false;
}

/// Starts or restarts the coordinate sequence from the first point.
inline void xysmStartSequence() {
    if (xysmTargetPoints.empty()) {
        return; // Nothing to run
    }
    xysmCurrentPointIndex = 0;
    xysmIsWaiting = false; // Start by moving, not waiting
    xysmSequenceActive = true;

    // Move to the first point
    auto& firstPoint = xysmTargetPoints[0];
    stepper0.moveTo(firstPoint.first);
    stepper1.moveTo(firstPoint.second);
}

/// [LOW-LEVEL] Must be called repeatedly in loop() to generate step pulses.
inline void xysmRun() {
    // Always run the low-level controllers
    stepper0.run();
    stepper1.run();
}

/// [HIGH-LEVEL] Must be called repeatedly in loop() to manage the sequence.
inline void xysmFollowTargetPoints() {
    // 1. Do nothing if sequence is not active or empty
    if (!xysmSequenceActive || xysmTargetPoints.empty()) {
        return;
    }

    // 2. If in "Waiting" state
    if (xysmIsWaiting) {
        // Check if wait time has elapsed
        unsigned long waitDuration = xysmWaitTimes_us[xysmCurrentPointIndex];
        
        if (micros() - xysmWaitStartTime_us >= waitDuration) {
            // Wait finished
            xysmIsWaiting = false;

            // 3. Get next point (LOOPING)
            xysmCurrentPointIndex = (xysmCurrentPointIndex + 1) % xysmTargetPoints.size();
            
            // 4. Command move to the next point
            auto& nextPoint = xysmTargetPoints[xysmCurrentPointIndex];
            stepper0.moveTo(nextPoint.first);
            stepper1.moveTo(nextPoint.second);
        }
        // (If not finished waiting, do nothing)

    } 
    // 5. If in "Moving" state
    else {
        // Check if both motors have arrived
        if (stepper0.distanceToGo() == 0 && stepper1.distanceToGo() == 0) {
            // Arrived at target!
            // Switch to "Waiting" state
            xysmIsWaiting = true;
            xysmWaitStartTime_us = micros(); // Start wait timer
        }
        // (If not arrived, do nothing, just let xysmRun() work)
    }
}


#endif // __XY_STEP_MOTOR_WRAP_H__