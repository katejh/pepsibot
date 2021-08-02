#include "Robot.h"
#include "Pin.h"
#include <Adafruit_SSD1306.h>

Robot::Robot()
{
    tapeFollower = TapeFollower();
}

/**
 * Set up all robot stuff
 */ 
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
  myServo.write(180);
}

void Robot::pickupSetup()
{
  pinMode(FLAPPER, OUTPUT);

  double percentage = 0.40;
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

/**
 * Drive straight forward
 * @param speed value from 0-1023
 */ 
void Robot::drive(int speed)
{
  driveLeft(speed);
  driveRight(speed);
}

/**
 * Control speed of left wheel
 * @param speed value from 0-1023
 */ 
void Robot::driveLeft(int speed)
{
    // value 0-1023
    int actual_value = speed + torque_vs_pwm_offset;
    if (actual_value > 1023) actual_value = 1023;
    if (actual_value < 0 || speed == 0) actual_value = 0;
    pwm_start(MOTOR_L, motor_freq, actual_value * 4095 / 1023, RESOLUTION_12B_COMPARE_FORMAT);
}

/**
 * Control speed of right wheel
 * @param speed value from 0-1023
 */ 
void Robot::driveRight(int speed)
{
    // value 0-1023
    int actual_value = speed + torque_vs_pwm_offset;
    if (actual_value > 1023) actual_value = 1023;
    if (actual_value < 0 || speed == 0) actual_value = 0;
    pwm_start(MOTOR_R, motor_freq, actual_value * 4095 / 1023, RESOLUTION_12B_COMPARE_FORMAT);
}

/**
 * Stop the robot
 */ 
void Robot::stop()
{
  drive(0);
}

/**
 * Activate dropoff function
 */ 
void Robot::dropOff()
{
  myServo.write(180);
  delay(2000);
  for (int i = 180; i >= 140; i -= 1)
  {
      myServo.write(i);
      delay(15);
  }

  for (int i = 140; i >= 70; i -= 5) {
    myServo.write(i);
    delay(5);
  }

  delay(2000);

  for (int i = 70; i <= 180; i += 5)
  {
      myServo.write(i);
      delay(25);
  }
}

/**
 * Let robot drive autonomously by following a tape path.
 * Assumes a white floor with black electrical tape path.
 */ 
void Robot::followTape()
{
    int pid_error = tapeFollower.getPidError();
    // comment out line below after speed testing is done, and set neutral_speed in Robot.h
    int neutral_speed = analogRead(KI_ADJUSTOR);

    driveLeft(neutral_speed - pid_error);
    driveRight(neutral_speed + pid_error);
}

/**
 * Control sweeper motor speed
 * @param speed value from 0-1023
 */ 
void Robot::sweep(int speed)
{
  double percentage = (double)speed / 1023;
  pwm_start(FLAPPER, flapper_freq, 4095 * percentage, RESOLUTION_12B_COMPARE_FORMAT);
}
