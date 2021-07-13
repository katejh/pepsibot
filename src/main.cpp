#include <Wire.h>
#include <Adafruit_SSD1306.h>

// IR sensor
#define IRSENSOR_FRONT PA_6
#define IRSENSOR_BACK PA_7

// comparators (for checking if rv sensors are within a certain range)
#define FRONT_COMPARATOR PB13
#define BACK_COMPARATOR PB12

// motor
#define MOTOR_A PA_8
#define MOTOR_B PA_9
#define MOTORFREQ 100

// sending tape values to comparators
#define TAPE_VAL_MIN_PIN PA_2
#define TAPE_VAL_MAX_PIN PA_3

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

void setupMotor()
{
  pinMode(MOTOR_A, OUTPUT);
  pinMode(MOTOR_B, OUTPUT);
}

void setupTapeValuePins()
{
  pinMode(TAPE_VAL_MIN_PIN, INPUT_ANALOG);
  pinMode(TAPE_VAL_MAX_PIN, INPUT_ANALOG);
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

/**
 * Function for prototyping the tape reading sensors
 */
void prototypeSensors() {
  int reading_left = analogRead(IRSENSOR_BACK);
  int reading_right = analogRead(IRSENSOR_FRONT);

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
  int reading_front = analogRead(IRSENSOR_FRONT);
  int reading_back = analogRead(IRSENSOR_BACK);
  int reading_front_comp = digitalRead(FRONT_COMPARATOR);
  int reading_back_comp = digitalRead(BACK_COMPARATOR);
  int tape_val_min = analogRead(TAPE_VAL_MIN_PIN);
  int tape_val_max = analogRead(TAPE_VAL_MAX_PIN);

  printToDisplay("Front sensor: " + String(reading_front) 
    + "\nBack sensor: " + String(reading_back)
    + "\nFront comparator: " + String(reading_front_comp)
    + "\nBack comparator: " + String(reading_back_comp)
    + "\nMin: " + String(tape_val_min) + " Max: " + String(tape_val_max)
  );

  if (reading_front_comp == LOW && reading_back_comp == LOW) {
    adjustMotor(511);
  }

  // wait 3 seconds
  delay(3000);
}

// main
void setup()
{
  setupReturnVehicleSensors();
  setupMotor();
  // make sure setupDisplay is last because for some reason you gotta call it after all pinModes are done 
  setupDisplay();

  // interrupts
  attachInterrupt(digitalPinToInterrupt(FRONT_COMPARATOR), prototypeReturnVehicleSensing, LOW);
}

void loop()
{
  adjustMotor(600);
}