#ifndef __BLE_BUZZER_SERVICE_H__
#define __BLE_BUZZER_SERVICE_H__

#define BUZZER_PWM_PERIOD 100

const static int BUZZER_MODE[10][2] {
    // cycle, t_on
    {1000, 800},
    {1000, 500},
    {1000, 300},
    {1000, 200},
    {1000, 100},
    {500, 300},
    {500, 200},
    {500, 100},
    {200, 100},
    {100, 50}
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
    
    void set_status(int32_t new_status) {
        int32_t new_state = getMode()*10 + getVolume()*100;
        new_state += new_status % 10;
        updateState(new_state);
    }
    void set_mode(int32_t new_mode) {
        int32_t new_state = getStatus() + getVolume()*100;
        new_state += (new_mode % 10) * 10;
        updateState(new_state);
    }
    void set_volume(int32_t new_volume) {
        int32_t new_state = getStatus() + getMode()*10;
        new_state += (new_volume % 100) * 100;
        updateState(new_state);
    }

private:
    BLE                                 &ble;
    ReadWriteGattCharacteristic<int32_t>   stateChar;

    int32_t state;
};
 
#endif /* #ifndef __BLE_BUZZER_SERVICE_H__ */