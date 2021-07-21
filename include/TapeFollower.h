class TapeFollower
{
    public:
        TapeFollower();
        int getPidError();
        bool isTapeReadingValue(int value);

    private:
        int last_error;
        int total_i;
        int last_error_timesteps;
        int current_error_timesteps;
        int neutral_motor_speed;
        int calculateError();
};