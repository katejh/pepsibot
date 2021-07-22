#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include "Robot.h"
#include "Pin.h"

#define MOTORFREQ 100

#define FLAPPER_FREQ 200

// display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // This display does not have a reset pin accessible
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// state machine
enum STATE {
  SKYCRANE,
  DRIVING,
  DROPOFF
};

STATE state = STATE::DRIVING;

Servo myServo;
Robot robot = Robot();

void printToDisplay(String text)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}

void servoSetup()
{
  myServo.attach(SERVO);
}

void pickupSetup()
{
  pinMode(FLAPPER, OUTPUT);

  double percentage = 0.2;
  pwm_start(FLAPPER, FLAPPER_FREQ, 4095 * percentage, RESOLUTION_12B_COMPARE_FORMAT);
}

void setupTapeSensors()
{
  pinMode(TAPESENSOR_LEFT, INPUT_ANALOG);
  pinMode(TAPESENSOR_RIGHT, INPUT_ANALOG);
}

void setupReturnVehicleSensors()
{
  pinMode(TAPESENSOR_RV, INPUT_ANALOG);
  pinMode(RV_COMPARATOR, INPUT_PULLUP);
}

void setupDisplay()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
}

void setupMotors()
{
  pinMode(MOTOR_L, OUTPUT);
  pinMode(MOTOR_R, OUTPUT);
}


void setupPtmtInputs()
{
  pinMode(KP_ADJUSTOR, INPUT_ANALOG);
  pinMode(KI_ADJUSTOR, INPUT_ANALOG);
  pinMode(KD_ADJUSTOR, INPUT_ANALOG);
  pinMode(TAPE_MIN_ADJUSTOR, INPUT_ANALOG);
}

/**
 * Take a value from 0 - 1023
 * 511 = stop
void adjustMotor(int adjust)
{
  // one duty cycle is divided into 4095 tics
  // analog values between 0 and 1023 => 0 to 3.3 V

  // printToDisplay(String(freqAdjustor));
  if (adjust > 511) {
    pwm_start(MOTOR_A, MOTORFREQ, (int)((adjust-512)*4095/511), RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(MOTOR_B, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
  } else {
    pwm_start(MOTOR_A, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
    pwm_start(MOTOR_B, MOTORFREQ, (int)((511 - adjust)*4095/511), RESOLUTION_12B_COMPARE_FORMAT);
  }
}
*/

void adjustLeftMotor(int value)
{
  // value 0-1023
  int offset = 0; // offset value to linearize torque vs pwm since function is not exactly linear due to friction
  int actual_value = value + offset;
  if (actual_value > 1023) actual_value = 1023;
  if (actual_value < 0 || value == 0) actual_value = 0;
  pwm_start(MOTOR_L, MOTORFREQ, actual_value * 4095 / 1023, RESOLUTION_12B_COMPARE_FORMAT);
}

void adjustRightMotor(int value)
{
  // value 0-1023
  int offset = 0; // offset value to linearize torque vs pwm since function is not exactly linear due to friction
  int actual_value = value + offset;
  if (actual_value > 1023) actual_value = 1023;
  if (actual_value < 0 || value == 0) actual_value = 0;
  pwm_start(MOTOR_R, MOTORFREQ, actual_value * 4095 / 1023, RESOLUTION_12B_COMPARE_FORMAT);
}

/**
 * Checks that an analog reading value is potentially tape
 * Expected that input is a normal analog reading of value 0 - 1023
 */ 
bool isTapeReadingValue(int reading)
{
  int tape_value_min = analogRead(TAPE_MIN_ADJUSTOR); // check this value before running the code

  if (reading >= tape_value_min){
    return true;
  } else {
    return false;
  }
}

/*
int last_error = 0;
int total_i = 0;
int last_error_timesteps = 0;
int current_error_timesteps = 0;
void prototypeTapeFollowingPid()
{
  int neutral_motor_value = 200; // regular speed for motor while driving straight

  int reading_left = analogRead(TAPESENSOR_LEFT);
  int reading_right = analogRead(TAPESENSOR_RIGHT);

  /*
  1 1 error = 0
  0 1 error = -1
  1 0 error = 1
  0 0 error = -5 if last error -1, 5 if last 1 (5 comes from ratio of one sensor off tape vs both sensors off tape. CHECK ROBOT FOR ACTUAL PROPORTIONS THIS IS JUST AN EXAMPLE)

  int error = 0;
  if (isTapeReadingValue(reading_left) && isTapeReadingValue(reading_right)){
    error = 0;
  } else if (!isTapeReadingValue(reading_left) && isTapeReadingValue(reading_right)) {
    error = -1;
  } else if (isTapeReadingValue(reading_left) && !isTapeReadingValue(reading_right)) {
    error = 1;
  } else if (last_error == -1 || last_error == -5) {
    error = -5;
  } else if (last_error == 1 || last_error == 5) {
    error = 5;
  }

  if (error != last_error) {
    last_error_timesteps = current_error_timesteps;
    current_error_timesteps = 0;
  }
  current_error_timesteps++;

  int k_p = analogRead(KP_ADJUSTOR);
  int k_i = analogRead(KI_ADJUSTOR) * 100 / 1023; // cap it at 100
  int k_d = analogRead(KD_ADJUSTOR) * 100 / 1023;

  int p = k_p * error;
  int i = k_i * (total_i + error);
  int d = k_d * ((float)(error - last_error) / (last_error_timesteps + current_error_timesteps));

  int pid_error = p + i + d;

  adjustLeftMotor(neutral_motor_value - pid_error);
  adjustRightMotor(neutral_motor_value + pid_error);

  total_i += i;
  last_error = error;

  printToDisplay(
    "min:" + String(analogRead(TAPE_MIN_ADJUSTOR)) + "\n"
    + "L:" + String(reading_left) + " R:" + String(reading_right) + "\n"
    + "k_p:" + String(k_p) + " k_i:" + String(k_i) + " k_d:" + String(k_d) + "\n"
    + "p:" + String(p) + " i:" + String(i) + " d:" + String(d) + "\n"
    + "Error:" + String(error)
  );
}
*/

/**
 * Function for prototyping the tape reading sensors
 */
void prototypeSensors() {
  int reading_left = analogRead(TAPESENSOR_LEFT);
  int reading_right = analogRead(TAPESENSOR_RIGHT);

  int reading_rv = analogRead(TAPESENSOR_RV);

  printToDisplay(
    "Left sensor: " + String(reading_left)
    + "\nRight sensor: " + String(reading_right)
    + "\nRV sensor: " + String(reading_rv)
  );
}

/**
 * Function for testing torque vs PWM
 * Since the function is not exactly linear, we need to find what values to map
 * for it to be linear
 */ 
void testTorqueVsPWM()
{
  int reading0 = analogRead(KP_ADJUSTOR);
  int reading1 = analogRead(KI_ADJUSTOR);

  //int dutycycle0 = reading0 * 100 / 1023; // percentage 0-100
  //int dutycycle1 = reading1 * 100 / 1023;

  printToDisplay("Reading 0: " + String(reading0)
    + "\nReading 1: " + String(reading1)
  );
  // adjust left and right motors appropriately
  adjustLeftMotor(reading0);
  adjustRightMotor(reading0);
}

/*
void adjustMotor(int duty) {
  int offset = 0; // offset value to linearize torque vs pwm since function is not exactly linear due to friction
  int actual_duty = duty + offset;
  if (actual_duty > 100) actual_duty = 100;
  pwm_start(MOTOR_PIN, MOTORFREQ, actual_duty * 4095 / 100, RESOLUTION_12B_COMPARE_FORMAT);
}
*/

void dropoffFunction()
{
  myServo.write(70);
  delay(7000);
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

  state = STATE::DRIVING;
}

void prototypeReturnVehicleSensing() {
  if (TapeFollower::isTapeReadingValue(analogRead(TAPESENSOR_LEFT)) && TapeFollower::isTapeReadingValue(TAPESENSOR_RIGHT)) {
    state = STATE::DROPOFF;
  }
}

void tapeFollowingPid()
{
  int neutral_motor_value = 60; // regular speed for motor while driving straight

  int pid_error = robot.tapeFollower.getPidError();

  adjustLeftMotor(neutral_motor_value - pid_error);
  adjustRightMotor(neutral_motor_value + pid_error);
}

// main
void setup()
{
  robot.setup();
  // make sure setupDisplay is last because for some reason you gotta call it after all pinModes are done 
  setupDisplay();

  // interrupts
  attachInterrupt(digitalPinToInterrupt(RV_COMPARATOR), prototypeReturnVehicleSensing, HIGH);
}

void loop()
{
  printToDisplay(
    "RV comparator: " + String(digitalRead(RV_COMPARATOR))
    + "\n"
    + "tape min: " + String(analogRead(TAPE_MIN_ADJUSTOR))
    + "\n"
    + "L:" + String(analogRead(TAPESENSOR_LEFT)) + " R:" + String(analogRead(TAPESENSOR_RIGHT))
    + " RV:" + String(analogRead(TAPESENSOR_RV))
    + "\n"
    + "kp:" + analogRead(KP_ADJUSTOR) + " ki:" + analogRead(KI_ADJUSTOR) + " kd:" + analogRead(KD_ADJUSTOR)
    + "\n"
    + "error:" + String(robot.tapeFollower.error)
  );
  
  switch(state) {
    case STATE::SKYCRANE:
      break;
    case STATE::DRIVING:
      robot.followTape();
      break;
    case STATE::DROPOFF:
      robot.dropOff();
      state = STATE::DRIVING;
      break;
  }
}