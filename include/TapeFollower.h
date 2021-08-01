#ifndef TAPEFOLLOWER_H
#define TAPEFOLLOWER_H

class TapeFollower
{
    public:
        TapeFollower();
        int getPidError();
        static bool isTapeReadingValue(int value);
        int error;

    private:
        int last_error;
        int total_i;
        int last_error_timesteps;
        int current_error_timesteps;
        int neutral_motor_speed;
        int calculateError();
};

#endif
