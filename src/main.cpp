#include <Wire.h>
#include <Adafruit_SSD1306.h>

// IR sensor
#define IRSENSOR_RIGHT PA_6
#define IRSENSOR_LEFT PA_7

// PID adjustor
#define K_P PA_3
#define K_I PA_4
#define K_D PA_5

// motor
#define MOTOR_A PA_0
#define MOTOR_B PA_1
#define MOTORFREQ 100

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

void setupIRSensors()
{
  pinMode(IRSENSOR_LEFT, INPUT_ANALOG);
  pinMode(IRSENSOR_RIGHT, INPUT_ANALOG);
}

void setupDisplay()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.display();
}

void setupMotor()
{
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
}

/**
 * Take a value from 0 - 1023
 * 511 = stop
 */
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

/*
int last_error = 0;
int last_i = 0;
int max_i = 100;

void pidTime()
{
  // this algorithm sucks sorry
  int reading_left = analogRead(IRSENSOR_LEFT);
  int reading_right = analogRead(IRSENSOR_RIGHT);
  // int set = 500;

  int error = 0; // reading - set;
  last_error = error;

  int kp = 1;
  int ki = 0;
  int kd = 1;

  int p = kp*error;
  int i = ki*error + last_i;
  if (i > max_i) {
    i = max_i;
  } else if (i < -max_i) {
    i = -max_i;
  }
  last_i = i;
  int d = kd*(error-last_error);

  int g = p+i+d;

  adjustMotor(g + 511);
}
*/

/**
 * Function for prototyping the tape reading sensors
 */
void prototypeSensors() {
  int reading_left = analogRead(IRSENSOR_LEFT);
  int reading_right = analogRead(IRSENSOR_RIGHT);

  // experimentally found normal around 30 tape around 40
  // subject to change for later tests
  int tape_reading_threshold = 40;

  printToDisplay(
    "Left sensor: " + String(reading_left) 
    + "\nRight sensor: " + String(reading_right)
  );

  if (reading_left >= tape_reading_threshold) {
    // do something lol
  } else if (reading_right >= tape_reading_threshold) {
    // do something lol
  } else {
    adjustMotor(511);
  }
}

void prototypeReturnVehicleSensing() {
  // experimentally found values
  // cardboard is around 30, electrical tape is around 50 or 60, normal light is >100
  int tape_reading_min = 50;
  int tape_reading_max = 80;
  int reading = analogRead(IRSENSOR_RIGHT);

  printToDisplay("Sensor reading: " + String(reading));

  if (reading <= tape_reading_max && reading >= tape_reading_min) {
    adjustMotor(511);
  } else {
    // turn on motor - value chosen randomly for now
    adjustMotor(600);
  }
}

// main
void setup()
{
  setupIRSensors();
  setupMotor();
  // make sure setupDisplay is last because for some reason you gotta call it after all pinModes are done 
  setupDisplay(); 
}

void loop()
{
  prototypeReturnVehicleSensing();
}