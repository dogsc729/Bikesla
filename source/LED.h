// Brightness of lightness in PWM.  unit: ms
#include <string>

#define LED_FLASH_MODE_LENGTH 5
#define LED_DUTY_MODE_LENGTH 5
#define LED_PERIOD_MODE_LENGTH 5
#define LED_CYCLE_MAX 2000

static int LED_DUTY_MODE[LED_DUTY_MODE_LENGTH] {
    0,
    100,
    200,
    500,
    800,
};
static int LED_PERIOD_MODE[LED_PERIOD_MODE_LENGTH] {
    100,
    200,
    500,
    1000,
    2000,
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
        _pwm_period = 1000;
        _pwm_duty = 500;
        _flash_mode = 0;
    }
    
    bool get_status() { return _status; }
    void turn_on() { _status = true; }
    void turn_off() { _status = false; }

    int get_pwm_period() { return _pwm_period; }
    float get_pwm_period_s() { return float(_pwm_period) / 1000; }
    void set_pwm_period(int new_period) { _pwm_period = new_period; }
    int get_pwm_duty() { return _pwm_duty; }
    float get_pwm_duty_s() { return float(_pwm_duty) / 1000; }
    void set_pwm_duty(int new_duty) { _pwm_duty = new_duty; }


    void next_flash_mode() {
        _flash_mode = (++_flash_mode) % LED_FLASH_MODE_LENGTH;  // Flash mode length
    }
    int get_flash_mode() { return _flash_mode; }
    void set_flash_mode(int new_mode) { _flash_mode = new_mode; }

    int get_cycle() { return LED_FLASH_MODE[_flash_mode][0]; }
    int get_t_on() { return LED_FLASH_MODE[_flash_mode][1]; }

    int ble_get_int() {
        int32_t ret = 0;
        if (_status) ret += 1;
        ret += _flash_mode*10;
        for (int i=0; i<LED_PERIOD_MODE_LENGTH; ++i)
            if (LED_PERIOD_MODE[i] == _pwm_period) ret += i*100;
        for (int i=0; i<LED_DUTY_MODE_LENGTH; ++i)
            if (LED_DUTY_MODE[i] == _pwm_duty) ret += i*1000;
        return ret;
    }
    void ble_set_int(int32_t ble_int) {
        _status = ble_int % 10;
        _flash_mode = (ble_int / 10) % 10;
        _pwm_period = LED_PERIOD_MODE[(ble_int / 100) % 10];
        _pwm_duty = LED_DUTY_MODE[(ble_int / 1000) % 10];
    }


private:
    bool _status;
    int _flash_mode;
    int _pwm_period;
    int _pwm_duty;
};

