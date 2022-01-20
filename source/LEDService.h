#ifndef __BLE_LED_SERVICE_H__
#define __BLE_LED_SERVICE_H__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"

#include <cstdint>
#include <string>

#define LED_PWM_PERIOD 5
#define LED_CYCLE_MAX 2000

static int LED_MODE[10][2] {
    // cycle, t_on
    {2000, 2000},
    {2000, 1500},
    {2000, 1000},
    {2000, 500},
    {1000, 800},
    {1000, 500},
    {1000, 200},
    {500, 300},
    {200, 100},
    {100, 50}
};

class LEDService {
public:
    const static uint16_t SERVICE_UUID              = 0xC000;
    const static uint16_t STATE_CHARACTERISTIC_UUID = 0xC001;
 
    LEDService(BLE &_ble, int32_t initState) :
        ble(_ble),
        state(initState),
        stateChar(STATE_CHARACTERISTIC_UUID, &initState)
    {
        GattCharacteristic *charTable[] = {&stateChar};
        GattService         ledService(SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
 
        ble.gattServer().addService(ledService);
    }
 
    GattAttribute::Handle_t getValueHandle() const
    {
        return stateChar.getValueHandle();
    }

    void updateState(int32_t newState) {
        newState = newState % 10000;
        state = newState;
        ble.gattServer().write(
            stateChar.getValueHandle(),
            (uint8_t *)&newState,
            sizeof(int32_t)
        );
    }

    int32_t getState() { return state; }
    int32_t getStatus() { return state % 10; }
    int32_t getMode() { return (state / 10) % 10; }
    int32_t getLightness() { return (state / 100) % 100; }
    float getLightnessPercentage() { return float(getLightness()) / 100.0; }
    
    void setState(int32_t new_state) { state = new_state; }
    void setStatus(int32_t new_status) {
        int32_t new_state = getMode()*10 + getLightness()*100;
        new_state += new_status % 10;
        setState(new_state);
        // updateState(new_state);
    }
    void setMode(int32_t new_mode) {
        int32_t new_state = getStatus() + getLightness()*100;
        new_state += new_mode % 10;
        setState(new_state);
        // updateState(new_state);
    }
    void setLightness(int32_t new_lightness) {
        int32_t new_state = getStatus() + getMode()*10;
        new_state += new_lightness % 100;
        setState(new_state);
        // updateState(new_state);
    }


    int getT_cycle() { return LED_MODE[getMode()][0]; }
    int getT_on() { return LED_MODE[getMode()][1]; }
    int getT_off() { return LED_MODE[getMode()][0] - LED_MODE[getMode()][1]; }

    // int ble_get_int() {
    //     int32_t ret = 0;
    //     ret += _status;
    //     ret += _mode*10;
    //     ret += _lightness*100;
    //     return ret;
    // }
    // void ble_set_int(int32_t ble_int) {
    //     _status = ble_int % 10;
    //     _mode = (ble_int / 10) % 10;
    //     _lightness = (ble_int / 100) % 100;
    // }

private:
    BLE                                 &ble;
    ReadWriteGattCharacteristic<int32_t>   stateChar;

    int32_t state;
};
 
#endif