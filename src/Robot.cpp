#include "Robot.h"
#include "Pin.h"
#include <Adafruit_SSD1306.h>

Robot::Robot()
{
    tapeFollower = TapeFollower();
}

void Robot::setup()
{
    setupPtmtInputs();
    setupTapeSensors();
    setupReturnVehicleSensors();
    setupMotors();
    pickupSetup();
    servoSetup();
}

void Robot::servoSetup()
{
  myServo.attach(SERVO);
}

void Robot::pickupSetup()
{
  pinMode(FLAPPER, OUTPUT);

  double percentage = 0.60;
  pwm_start(FLAPPER, flapper_freq, 4095 * percentage, RESOLUTION_12B_COMPARE_FORMAT);
}

void Robot::setupTapeSensors()
{
  pinMode(TAPESENSOR_LEFT, INPUT_ANALOG);
  pinMode(TAPESENSOR_RIGHT, INPUT_ANALOG);
}

void Robot::setupReturnVehicleSensors()
{
  pinMode(TAPESENSOR_RV, INPUT_ANALOG);
  pinMode(RV_COMPARATOR, INPUT_PULLUP);
}

void Robot::setupMotors()
{
  pinMode(MOTOR_L, OUTPUT);
  pinMode(MOTOR_R, OUTPUT);
}

void Robot::setupPtmtInputs()
{
  pinMode(KP_ADJUSTOR, INPUT_ANALOG);
  pinMode(KI_ADJUSTOR, INPUT_ANALOG);
  pinMode(KD_ADJUSTOR, INPUT_ANALOG);
  pinMode(TAPE_MIN_ADJUSTOR, INPUT_ANALOG);
}

void Robot::driveLeft(int speed)
{
    // value 0-1023
    int actual_value = speed + torque_vs_pwm_offset;
    if (actual_value > 1023) actual_value = 1023;
    if (actual_value < 0 || speed == 0) actual_value = 0;
    pwm_start(MOTOR_L, motor_freq, actual_value * 4095 / 1023, RESOLUTION_12B_COMPARE_FORMAT);
}

void Robot::driveRight(int speed)
{
    // value 0-1023
    int actual_value = speed + torque_vs_pwm_offset;
    if (actual_value > 1023) actual_value = 1023;
    if (actual_value < 0 || speed == 0) actual_value = 0;
    pwm_start(MOTOR_R, motor_freq, actual_value * 4095 / 1023, RESOLUTION_12B_COMPARE_FORMAT);
}

void Robot::dropOff()
{
    driveLeft(0);
    driveRight(0);

    myServo.write(70);
    delay(2000);
    for (int i = 70; i >= 0; i -= 5)
    {
        myServo.write(i);
        delay(15);
    }

    delay(2000);

    for (int i = 0; i <= 70; i += 5)
    {
        myServo.write(i);
        delay(15);
    }
}

void Robot::followTape()
{
    int pid_error = tapeFollower.getPidError();

    driveLeft(neutral_speed - pid_error);
    driveRight(neutral_speed + pid_error);
}