#include "TapeFollower.h"
#include "Pin.h"
#include <Adafruit_SSD1306.h>

TapeFollower::TapeFollower()
{
    last_error = 0;
    total_i = 0;
    last_error_timesteps = 0;
    current_error_timesteps = 0;
    neutral_motor_speed = 200;
}

int TapeFollower::getPidError()
{
    int error = calculateError();
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

    total_i += i;
    last_error = error;

    return pid_error;
}

int TapeFollower::calculateError()
{
    /*
    1 1 error = 0
    0 1 error = -1
    1 0 error = 1
    0 0 error = -5 if last error -1, 5 if last 1 (5 comes from ratio of one sensor off tape vs both sensors off tape. CHECK ROBOT FOR ACTUAL PROPORTIONS THIS IS JUST AN EXAMPLE)
    */

    int reading_left = analogRead(TAPESENSOR_LEFT);
    int reading_right = analogRead(TAPESENSOR_RIGHT);
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

    return error;
}

bool TapeFollower::isTapeReadingValue(int value)
{
    int tape_value_min = analogRead(TAPE_MIN_ADJUSTOR); // check this value before running the code

    if (value >= tape_value_min){
        return true;
    } else {
        return false;
    }
}
