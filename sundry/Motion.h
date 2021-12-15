class Motion {
public:
    struct Status {
        const static int REST = 0;
        const static int RIDE = 1;
        const static int STOP = 2;
    };

    float get_speed() { return speed; }
    void set_speed(float new_speed) { speed = new_speed; }
    bool get_status() { return status; }
    void mute() { status = false; }
    void ring() { status = true; }

private:
    // TODO: x, y, z axis?
    float speed = 0;
    bool status = false;
    
    float steal_time_threshold = 10.0;
};