#ifndef ROBOT_H
#define ROBOT_H

#include "TapeFollower.h"
#include <Servo.h>

class Robot
{
    public:
        Robot();
        void setup();
        void drive(int speed);
        void driveLeft(int speed);
        void driveRight(int speed);
        void stop();
        void dropOff();
        void followTape();
        void sweep(int speed);
        TapeFollower tapeFollower;

    private:
        static const int neutral_speed = 100;
        static const int torque_vs_pwm_offset = 165; // offset value to linearize torque vs pwm since function is not exactly linear due to friction
        static const int motor_freq = 100;
        static const int flapper_freq = 1000;
        Servo myServo;
        void servoSetup();
        void pickupSetup();
        void setupTapeSensors();
        void setupReturnVehicleSensors();
        void setupMotors();
        void setupPtmtInputs();
};

#endif
