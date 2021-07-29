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

Robot robot = Robot();

void printToDisplay(String text)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}

void setupDisplay()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.display();
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
  int reading = analogRead(KP_ADJUSTOR);

  printToDisplay("Speed: " + String(reading));
  
  robot.drive(reading);
}

void prototypeReturnVehicleSensing() {
  state = STATE::DROPOFF;
}

// main
void setup()
{
  robot.setup();
  // make sure setupDisplay is last because for some reason you gotta call it after all pinModes are done 
  setupDisplay();

  // interrupts
  attachInterrupt(digitalPinToInterrupt(RV_COMPARATOR), prototypeReturnVehicleSensing, RISING);
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
      if (robot.tapeFollower.error == 0) {
        printToDisplay("dropping off");
        robot.stop();
        robot.dropOff();
      }
      state = STATE::DRIVING;
      break;
  }
}
