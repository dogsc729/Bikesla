#ifndef __BLE_Motion_SERVICE_H__
#define __BLE_Motion_SERVICE_H__

#include <cstdint>
#include <string>

#define SPEED_SAMPLE_T 10
#define ACC_SAMPLE_T 100

#define CIRCUMFERENCE 2 * 3.14159 * 0.0006

class MotionService {
public:
    const static uint16_t SERVICE_UUID              = 0xD000;
    const static uint16_t STATE_CHARACTERISTIC_UUID = 0xD001;
    const static uint16_t SPEED_CHARACTERISTIC_UUID = 0xD002;

    MotionService(BLE &_ble, int32_t initState, int32_t initSpeed) :
        ble(_ble),
        state(initState),
        stateChar(STATE_CHARACTERISTIC_UUID, &initState),
        speed(initSpeed),
        speedChar(SPEED_CHARACTERISTIC_UUID, &initSpeed, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
    {
        GattCharacteristic *charTable[] = {&stateChar, &speedChar};
        GattService motionService(
            SERVICE_UUID,
            charTable,
            sizeof(charTable) / sizeof(GattCharacteristic *)
        );

        ble.gattServer().addService(motionService);
    }
 
    GattAttribute::Handle_t getValueHandle() const
    {
        return stateChar.getValueHandle();
    }

    void updateState(int32_t newState) {
        state = newState;
        ble.gattServer().write(
            stateChar.getValueHandle(),
            (uint8_t *)&newState,
            sizeof(int32_t)
        );
    }

    void updateSpeed(int32_t new_speed) {
        speed = new_speed;
        ble.gattServer().write(
            speedChar.getValueHandle(),
            (uint8_t *)&new_speed,
            sizeof(int32_t)
        );
    }

    int32_t getState() { return state; }
    int32_t getStatus() { return state % 10; }
    int32_t getPatience() { return state / 10; }
    bool locked() { return getStatus() == 0; }

    void setStatus(int32_t new_status) {
        int32_t new_state = getPatience() * 10;
        new_state += new_status % 10;
        updateState(new_state);
    }
    void setPatience(int32_t new_patience) {
        int32_t new_state = getStatus();
        new_state += new_patience * 10;
        updateState(new_state);
    }

    bool unsafeMove() {
        int32_t patience = getPatience();
        updateState(state - 10);
        return patience < 10;
    }

    int getSpeed() { return speed; }

private:
    BLE                                 &ble;
    ReadWriteGattCharacteristic<int32_t> stateChar;
    ReadOnlyGattCharacteristic<int32_t> speedChar;

    int32_t state;
    int32_t speed;
};
 
#endif