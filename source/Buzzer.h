class Buzzer {
public:
    float get_volume() { return volume; }
    void set_volume(float new_volume) { volume = new_volume; }
    bool get_status() { return status; }
    void mute() { status = false; }
    void ring() { status = true; }

private:
    float volume = 0;
    bool status = false;
};