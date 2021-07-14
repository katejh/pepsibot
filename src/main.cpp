#include <Wire.h>
#include <Adafruit_SSD1306.h>

// IR sensor
#define TAPESENSOR_LEFT PA_0
#define TAPESENSOR_RIGHT PA_1

#define IRSENSOR_FRONT PA_3
#define IRSENSOR_BACK PA_7

// Potentiometer inputs
#define KP_ADJUSTOR PA_4
#define KI_ADJUSTOR PA_5
#define KD_ADJUSTOR PA_6
#define TAPE_BOTTOM_MIN_ADJUSTOR PA_2

// motors
#define MOTOR_LA PA_8
#define MOTOR_LB PA_9
#define MOTOR_RA PA_10
#define MOTOR_RB PA_11

// comparators (for checking if rv sensors are within a certain range)
#define FRONT_COMPARATOR PB13
#define BACK_COMPARATOR PB12

// motor
#define MOTOR_A PA_8
#define MOTOR_B PA_9
#define MOTORFREQ 100

// sending tape values to comparators
#define TAPE_SIDE_MIN_ADJUSTOR PB0
#define TAPE_SIDE_MAX_ADJUSTOR PB1

// display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // This display does not have a reset pin accessible
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void printToDisplay(String text)
{
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(text);
  display.display();
}

void setupTapeSensors()
{
  pinMode(TAPESENSOR_LEFT, INPUT_ANALOG);
  pinMode(TAPESENSOR_RIGHT, INPUT_ANALOG);
}

void setupReturnVehicleSensors()
{
  pinMode(IRSENSOR_BACK, INPUT_ANALOG);
  pinMode(IRSENSOR_FRONT, INPUT_ANALOG);
  pinMode(FRONT_COMPARATOR, INPUT_PULLUP);
  pinMode(BACK_COMPARATOR, INPUT_PULLUP);
}

void setupDisplay()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
}

void setupMotors()
{
  pinMode(MOTOR_LA, OUTPUT);
  pinMode(MOTOR_LB, OUTPUT);
  pinMode(MOTOR_RA, OUTPUT);
  pinMode(MOTOR_RB, OUTPUT);
}


void setupPtmtInputs()
{
  pinMode(KP_ADJUSTOR, INPUT_ANALOG);
  pinMode(KI_ADJUSTOR, INPUT_ANALOG);
  pinMode(KD_ADJUSTOR, INPUT_ANALOG);
  pinMode(TAPE_BOTTOM_MIN_ADJUSTOR, INPUT_ANALOG);
  pinMode(TAPE_SIDE_MIN_ADJUSTOR, INPUT_ANALOG);
  pinMode(TAPE_SIDE_MAX_ADJUSTOR, INPUT_ANALOG);
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
  // note if right motor off offset is 430
  int offset = 210; // offset value to linearize torque vs pwm since function is not exactly linear due to friction
  int actual_value = value + offset;
  if (actual_value > 1023) actual_value = 1023;
  if (actual_value < 0) actual_value = 0;
  pwm_start(MOTOR_LA, MOTORFREQ, actual_value * 4095 / 1023, RESOLUTION_12B_COMPARE_FORMAT);
  pwm_start(MOTOR_LB, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
}

void adjustRightMotor(int value)
{
  // value 0-1023
  // note if left motor off offset is 430
  int offset = 210; // offset value to linearize torque vs pwm since function is not exactly linear due to friction
  int actual_value = value + offset;
  if (actual_value > 1023) actual_value = 1023;
  if (actual_value < 0) actual_value = 0;
  pwm_start(MOTOR_RA, MOTORFREQ, actual_value * 4095 / 1023, RESOLUTION_12B_COMPARE_FORMAT);
  pwm_start(MOTOR_RB, MOTORFREQ, 0, RESOLUTION_12B_COMPARE_FORMAT);
}

/**
 * Checks that an analog reading value is potentially tape
 * Expected that input is a normal analog reading of value 0 - 1023
 */ 
bool isBottomTapeReadingValue(int reading)
{
  int tape_value_min = analogRead(TAPE_BOTTOM_MIN_ADJUSTOR); // check this value before running the code

  if (reading >= tape_value_min){
    return true;
  } else {
    return false;
  }
}

bool isSideTapeReadingValue(int reading)
{
  int tape_value_min = analogRead(TAPE_SIDE_MIN_ADJUSTOR);
  int tape_value_max = analogRead(TAPE_SIDE_MAX_ADJUSTOR);

  if (reading >= tape_value_min && reading <= tape_value_max) {
    return true;
  } else {
    return false;
  }
}

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
  */

  int error = 0;
  if (isBottomTapeReadingValue(reading_left) && isBottomTapeReadingValue(reading_right)){
    error = 0;
  } else if (!isBottomTapeReadingValue(reading_left) && isBottomTapeReadingValue(reading_right)) {
    error = -1;
  } else if (isBottomTapeReadingValue(reading_left) && !isBottomTapeReadingValue(reading_right)) {
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

  int k_p = analogRead(KP_ADJUSTOR) * 100 / 1023; // cap it at 100 for now
  int k_i = analogRead(KI_ADJUSTOR) * 100 / 1023;
  int k_d = analogRead(KD_ADJUSTOR) * 100 / 1023;

  int p = k_p * error;
  int i = k_i * (total_i + error);
  int d = k_d * ((float)(error - last_error) / (last_error_timesteps + current_error_timesteps));

  int pid_error = p + i + d;

  adjustLeftMotor(neutral_motor_value + pid_error);
  adjustRightMotor(neutral_motor_value - pid_error);

  total_i += i;
  last_error = error;

  /*
  printToDisplay(
    "min:" + String(analogRead(TAPE_BOTTOM_MIN_ADJUSTOR)) + "\n"
    + "L:" + String(reading_left) + " R:" + String(reading_right) + "\n"
    + "k_p:" + String(k_p) + " k_i:" + String(k_i) + " k_d:" + String(k_d) + "\n"
    + "p:" + String(p) + " i:" + String(i) + " d:" + String(d) + "\n"
    + "Error:" + String(error)
  );
  */
}

/**
 * Function for prototyping the tape reading sensors
 */
void prototypeSensors() {
  int reading_left = analogRead(TAPESENSOR_LEFT);
  int reading_right = analogRead(TAPESENSOR_RIGHT);

  int reading_back = analogRead(IRSENSOR_BACK);
  int reading_front = analogRead(IRSENSOR_FRONT);


  // experimentally found normal around 30 tape around 40
  // subject to change for later tests
  int tape_reading_threshold = 40;

  printToDisplay(
    "Left sensor: " + String(reading_left)
    + "\nRight sensor: " + String(reading_right)
    + "\nFront sensor: " + String(reading_front)
    + "\nBack sensor: " + String(reading_back)
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
  adjustRightMotor(reading1);
}

/*
void adjustMotor(int duty) {
  int offset = 0; // offset value to linearize torque vs pwm since function is not exactly linear due to friction
  int actual_duty = duty + offset;
  if (actual_duty > 100) actual_duty = 100;
  pwm_start(MOTOR_PIN, MOTORFREQ, actual_duty * 4095 / 100, RESOLUTION_12B_COMPARE_FORMAT);
}
*/

void prototypeReturnVehicleSensing() {
  int reading_front = analogRead(IRSENSOR_FRONT);
  int reading_back = analogRead(IRSENSOR_BACK);
  int reading_front_comp = digitalRead(FRONT_COMPARATOR);
  int reading_back_comp = digitalRead(BACK_COMPARATOR);
  int tape_val_min = analogRead(TAPE_SIDE_MIN_ADJUSTOR);
  int tape_val_max = analogRead(TAPE_SIDE_MAX_ADJUSTOR);

  printToDisplay("Front sensor: " + String(reading_front) 
    + "\nBack sensor: " + String(reading_back)
    + "\nFront comparator: " + String(reading_front_comp)
    + "\nBack comparator: " + String(reading_back_comp)
    + "\nMin: " + String(tape_val_min) + " Max: " + String(tape_val_max)
  );


  if (reading_front_comp == LOW && reading_back_comp == LOW) {
    adjustLeftMotor(0);
    adjustRightMotor(0);
  }

  // wait 3 seconds
  delay(3000);
  // activate dropoff
}

// main
void setup()
{
  setupPtmtInputs();
  setupTapeSensors();
  setupReturnVehicleSensors();
  setupMotors();
  // make sure setupDisplay is last because for some reason you gotta call it after all pinModes are done 
  setupDisplay();

  // interrupts
  //attachInterrupt(digitalPinToInterrupt(FRONT_COMPARATOR), prototypeReturnVehicleSensing, LOW);
}

void loop()
{
  printToDisplay(
    " side min:" + String(analogRead(TAPE_SIDE_MIN_ADJUSTOR))
    + " side max:" + String(analogRead(TAPE_SIDE_MAX_ADJUSTOR))
    + "\n"
    + " F:" + String(analogRead(IRSENSOR_FRONT)) + " B:" + String(analogRead(IRSENSOR_BACK))
    + "\n"
    + "Fcomp:" + String(analogRead(FRONT_COMPARATOR)) + " Bcomp:" + String(analogRead(BACK_COMPARATOR))
  );

  adjustLeftMotor(200);
  adjustRightMotor(200);
}