// Brightness of lightness in PWM.  unit: ms
#include <string>

#define LED_FLASH_MODE_LENGTH 4
#define LED_CYCLE_MAX 2000

enum class Light {
    OFF = 0,
    LOW = 250,
    MID = 500,
    HIGH = 750,
    MAX = 1000
};

static int LED_FLASH_MODE[LED_FLASH_MODE_LENGTH][2] {
    // cycle, t_on
    {2000, 1000},
    {1000, 500},
    {500, 300},
    {200, 100}
};

class LED {
public:
    LED () {
        _status = false;
        _pwm_period = 5;
        _pwm_duty = static_cast<int>(Light::MID);
        _flash_mode = 0;
    }
    
    void show_status() {
        printf("status: %d", _status);
        printf("flash mode: %d", _flash_mode);
        printf("pwm period: %d", _pwm_period);
        printf("pwm duty: %d", _pwm_duty);
    }

    bool get_status() { return _status; }
    void turn_on() { _status = true; }
    void turn_off() { _status = false; }

    int get_pwm_period() { return _pwm_period; }
    float get_pwm_period_s() { return float(_pwm_period) / 1000; }
    void set_pwm_period(int new_period) { _pwm_period = new_period; }
    int get_pwm_duty() { return _pwm_duty; }
    float get_pwm_duty_f() { return float(static_cast<int>(_pwm_duty)) / 1000; }
    void set_pwm_duty(int new_duty) { _pwm_duty = new_duty; }

    void next_flash_mode() {
        _flash_mode = (++_flash_mode) % LED_FLASH_MODE_LENGTH;  // Flash mode length
    }
    int get_cycle() { return LED_FLASH_MODE[_flash_mode][0]; }
    int get_t_on() { return LED_FLASH_MODE[_flash_mode][1]; }

    int ble_get_int() {
        int ret = 0;
        if (_status) ret += 1;
        ret += _flash_mode*10;
        ret += _pwm_period*100;
        ret += _pwm_duty*1000000;
        return ret;
    }
    void ble_set_int(int ble_int) {
        _status = ble_int % 10;
        _flash_mode = (ble_int / 10) % 10;
        _pwm_period = (ble_int / 100) % 1000;
        _pwm_duty = (ble_int / 1000000) % 1000;
        show_status();
    }


private:
    bool _status;
    int _flash_mode;
    int _pwm_period;
    int _pwm_duty;
};

