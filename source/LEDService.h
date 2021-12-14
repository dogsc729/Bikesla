/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#ifndef __BLE_LED_SERVICE_H__
#define __BLE_LED_SERVICE_H__

 
class LEDService {
public:
    const static uint16_t LED_SERVICE_UUID              = 0xC000;
    const static uint16_t LED_STATE_CHARACTERISTIC_UUID = 0xC001;
 
    LEDService(BLE &_ble, int initialValueForLEDCharacteristic) :
        ble(_ble), ledState(LED_STATE_CHARACTERISTIC_UUID, &initialValueForLEDCharacteristic)
    {
        GattCharacteristic *charTable[] = {&ledState};
        GattService         ledService(LED_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
 
        ble.gattServer().addService(ledService);
    }
 
    GattAttribute::Handle_t getValueHandle() const
    {
        return ledState.getValueHandle();
    }

    void updateLEDState(int newState) {
        ble.gattServer().write(ledState.getValueHandle(), (uint8_t *)&newState, sizeof(int));
    }


private:
    BLE                                 &ble;
    // [9-6][5-2][1][0] // max=2147483647
    // _duty, _period, _flash_mode, _status
    ReadWriteGattCharacteristic<int>   ledState;
};
 
#endif /* #ifndef __BLE_LED_SERVICE_H__ */