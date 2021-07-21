#ifndef ROBOT_H
#define ROBOT_H

#include "TapeFollower.h"
#include <Servo.h>

class Robot
{
    public:
        Robot();
        void setup();
        void driveLeft(int speed);
        void driveRight(int speed);
        void dropOff();
        void followTape();
        TapeFollower tapeFollower;

    private:
        static const int neutral_speed = 60;
        static const int torque_vs_pwm_offset = 166; // offset value to linearize torque vs pwm since function is not exactly linear due to friction
        static const int motor_freq = 100;
        static const int flapper_freq = 200;
        Servo myServo;
        void servoSetup();
        void pickupSetup();
        void setupTapeSensors();
        void setupReturnVehicleSensors();
        void setupMotors();
        void setupPtmtInputs();
};

#endif
