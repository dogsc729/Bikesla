#ifndef __BLE_BUZZER_SERVICE_H__
#define __BLE_BUZZER_SERVICE_H__

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "ble/GattServer.h"
#include <cstdint>

#define BUZZER_PWM_PERIOD 10
#define BUZZER_CYCLE_MAX 2000

const static int BUZZER_MODE[10][2] {
    // cycle, t_on
    {2000, 2000},
    {2000, 1500},
    {2000, 1000},
    {2000, 500},
    {1000, 800},
    {1000, 500},
    {1000, 200},
    {500, 400},
    {500, 300},
    {300, 100}
};


class BuzzerService {
public:
    const static uint16_t SERVICE_UUID              = 0xB000;
    const static uint16_t STATE_CHARACTERISTIC_UUID = 0xB001;

    BuzzerService(BLE &_ble, int32_t initState) :
        ble(_ble),
        state(initState),
        stateChar(STATE_CHARACTERISTIC_UUID, &initState)
    {
        GattCharacteristic *charTable[] = {&stateChar};
        GattService         buzzerService(SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
 
        ble.gattServer().addService(buzzerService);
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
            (uint8_t *)&state,
            sizeof(int32_t)
        );
    }

    int32_t getState() { return state; }
    int32_t getStatus() { return state % 10; }
    int32_t getMode() { return (state / 10) % 10; }
    int32_t getVolume() { return (state / 100) % 100; }
    float getVolumePercentage() { return float(getVolume()) / 100; }

    int getT_cycle() { return BUZZER_MODE[getMode()][0]; }
    int getT_on() { return BUZZER_MODE[getMode()][1]; }
    int getT_off() { return BUZZER_MODE[getMode()][0] - BUZZER_MODE[getMode()][1]; }
    
    void setState(int32_t new_state) { state = new_state; }
    void setStatus(int32_t new_status) {
        int32_t new_state = getMode()*10 + getVolume()*100;
        new_state += new_status % 10;
        setState(new_state);
        // updateState(new_state);
    }
    void setMode(int32_t new_mode) {
        int32_t new_state = getStatus() + getVolume()*100;
        new_state += (new_mode % 10) * 10;
        setState(new_state);
        // updateState(new_state);
    }
    void setVolume(int32_t new_volume) {
        int32_t new_state = getStatus() + getMode()*10;
        new_state += (new_volume % 100) * 100;
        setState(new_state);
        // updateState(new_state);
    }



private:
    BLE                                 &ble;
    ReadWriteGattCharacteristic<int32_t>   stateChar;

    int32_t state;
};
 
#endif /* #ifndef __BLE_BUZZER_SERVICE_H__ */