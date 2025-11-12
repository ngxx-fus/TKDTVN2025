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

// ===================================
// MOTOR PIN DEFINITIONS
// ===================================
// (-1 means disabled)
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

// ===================================
// LIMIT SWITCH PIN DEFINITIONS
// ===================================
#ifndef HM_XY_LIMIT0_UPPER
    #define HM_XY_LIMIT0_UPPER      -1
#endif
#ifndef HM_XY_LIMIT0_LOWER
    #define HM_XY_LIMIT0_LOWER      -1
#endif
#ifndef HM_XY_LIMIT1_UPPER
    #define HM_XY_LIMIT1_UPPER      -1
#endif
#ifndef HM_XY_LIMIT1_LOWER
    #define HM_XY_LIMIT1_LOWER      -1
#endif

// ===================================
// INTERNAL STATIC VARIABLES
// ===================================

static float xysmMaxSpeed = 400.0f; 
static float xysmMaxAcceleration = 100.0f; 

// --- Stepper Objects ---
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

// --- Coordinate Sequence Types ---
typedef std::pair<long, long> xysmPoint_t;

static std::vector<xysmPoint_t> xysmTargetPoints;
static std::vector<unsigned long> xysmWaitTimes_us;

// --- State Machine Variables ---
static size_t        xysmCurrentPointIndex = 0;
static bool          xysmIsWaiting = false;
static unsigned long xysmWaitStartTime_us = 0;
static bool          xysmSequenceActive = false;

// --- Limit Switch State ---
static bool xysmLimit0UpperValid = false;
static bool xysmLimit0LowerValid = false;
static bool xysmLimit1UpperValid = false;
static bool xysmLimit1LowerValid = false;

// ===================================
// FUNCTION IMPLEMENTATIONS (inline)
// ===================================

/// Initializes motors, speed, acceleration, and limit switches.
inline void xysmInit(float maxSpeed, float acceleration) {
    xysmMaxSpeed = maxSpeed;
    xysmMaxAcceleration = acceleration;

    stepper0.setMaxSpeed(xysmMaxSpeed);
    stepper0.setAcceleration(xysmMaxAcceleration);
    stepper0.setCurrentPosition(0);

    stepper1.setMaxSpeed(xysmMaxSpeed);
    stepper1.setAcceleration(xysmMaxAcceleration);
    stepper1.setCurrentPosition(0);

    // Init Limit Switches (Active Low)
    #if (HM_XY_LIMIT0_UPPER != -1)
        pinMode(HM_XY_LIMIT0_UPPER, INPUT_PULLUP);
        xysmLimit0UpperValid = true;
    #endif
    #if (HM_XY_LIMIT0_LOWER != -1)
        pinMode(HM_XY_LIMIT0_LOWER, INPUT_PULLUP);
        xysmLimit0LowerValid = true;
    #endif
    #if (HM_XY_LIMIT1_UPPER != -1)
        pinMode(HM_XY_LIMIT1_UPPER, INPUT_PULLUP);
        xysmLimit1UpperValid = true;
    #endif
    #if (HM_XY_LIMIT1_LOWER != -1)
        pinMode(HM_XY_LIMIT1_LOWER, INPUT_PULLUP);
        xysmLimit1LowerValid = true;
    #endif
}

/// Manually moves the motors by a relative distance (checks limits).
inline void xysmMove(long dx, long dy) {
    xysmSequenceActive = false; 

    // Check X-axis limits before commanding move
    if (xysmLimit0UpperValid && digitalRead(HM_XY_LIMIT0_UPPER) == LOW && dx > 0) {
        dx = 0; // Prevent moving further into upper limit
    }
    if (xysmLimit0LowerValid && digitalRead(HM_XY_LIMIT0_LOWER) == LOW && dx < 0) {
        dx = 0; // Prevent moving further into lower limit
    }
    stepper0.move(dx);

    // Check Y-axis limits before commanding move
    if (xysmLimit1UpperValid && digitalRead(HM_XY_LIMIT1_UPPER) == LOW && dy > 0) {
        dy = 0;
    }
    if (xysmLimit1LowerValid && digitalRead(HM_XY_LIMIT1_LOWER) == LOW && dy < 0) {
        dy = 0;
    }
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

/// Starts or restarts the coordinate sequence (checks limits).
inline void xysmStartSequence() {
    if (xysmTargetPoints.empty()) {
        return; // Nothing to run
    }
    xysmCurrentPointIndex = 0;
    xysmIsWaiting = false; 
    xysmSequenceActive = true;

    // Move to the first point, after checking limits
    auto& firstPoint = xysmTargetPoints[0];
    long nextX = firstPoint.first;
    long nextY = firstPoint.second;

    if (xysmLimit0UpperValid && digitalRead(HM_XY_LIMIT0_UPPER) == LOW && (nextX > stepper0.currentPosition())) {
        nextX = stepper0.currentPosition(); // Clamp target
    }
    if (xysmLimit0LowerValid && digitalRead(HM_XY_LIMIT0_LOWER) == LOW && (nextX < stepper0.currentPosition())) {
        nextX = stepper0.currentPosition(); // Clamp target
    }
    stepper0.moveTo(nextX);
    
    if (xysmLimit1UpperValid && digitalRead(HM_XY_LIMIT1_UPPER) == LOW && (nextY > stepper1.currentPosition())) {
        nextY = stepper1.currentPosition(); // Clamp target
    }
    if (xysmLimit1LowerValid && digitalRead(HM_XY_LIMIT1_LOWER) == LOW && (nextY < stepper1.currentPosition())) {
        nextY = stepper1.currentPosition(); // Clamp target
    }
    stepper1.moveTo(nextY);
}

/// [LOW-LEVEL] Must be called repeatedly, handles emergency stops from limits.
inline void xysmRun() {
    // Check for emergency stops (if logic tries to move into an active limit)
    if (xysmLimit0UpperValid && digitalRead(HM_XY_LIMIT0_UPPER) == LOW && (stepper0.targetPosition() > stepper0.currentPosition())) {
        stepper0.stop(); // Force stop
    }
    if (xysmLimit0LowerValid && digitalRead(HM_XY_LIMIT0_LOWER) == LOW && (stepper0.targetPosition() < stepper0.currentPosition())) {
        stepper0.stop(); // Force stop
    }
    
    if (xysmLimit1UpperValid && digitalRead(HM_XY_LIMIT1_UPPER) == LOW && (stepper1.targetPosition() > stepper1.currentPosition())) {
        stepper1.stop(); // Force stop
    }
    if (xysmLimit1LowerValid && digitalRead(HM_XY_LIMIT1_LOWER) == LOW && (stepper1.targetPosition() < stepper1.currentPosition())) {
        stepper1.stop(); // Force stop
    }

    // Always run the low-level controllers
    stepper0.run();
    stepper1.run();
}

/// [HIGH-LEVEL] Must be called repeatedly, manages sequence (checks limits).
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
            
            // 4. Command move to the next point (WITH LIMIT CHECKS)
            auto& nextPoint = xysmTargetPoints[xysmCurrentPointIndex];
            long nextX = nextPoint.first;
            long nextY = nextPoint.second;

            if (xysmLimit0UpperValid && digitalRead(HM_XY_LIMIT0_UPPER) == LOW && (nextX > stepper0.currentPosition())) {
                nextX = stepper0.currentPosition(); // Clamp target
            }
            if (xysmLimit0LowerValid && digitalRead(HM_XY_LIMIT0_LOWER) == LOW && (nextX < stepper0.currentPosition())) {
                nextX = stepper0.currentPosition(); // Clamp target
            }
            stepper0.moveTo(nextX);
            
            if (xysmLimit1UpperValid && digitalRead(HM_XY_LIMIT1_UPPER) == LOW && (nextY > stepper1.currentPosition())) {
                nextY = stepper1.currentPosition(); // Clamp target
            }
            if (xysmLimit1LowerValid && digitalRead(HM_XY_LIMIT1_LOWER) == LOW && (nextY < stepper1.currentPosition())) {
                nextY = stepper1.currentPosition(); // Clamp target
            }
            stepper1.moveTo(nextY);
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