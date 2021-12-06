#ifndef __BLE_ID_SERVICE_H__
#define __BLE_ID_SERVICE_H__

class IDService {
public:
    const static uint16_t ID_SERVICE_UUID              = 0xB000;
    const static uint16_t ID_STR_CHARACTERISTIC_UUID   = 0xB001;

    IDService(BLE &_ble, char* IDStrInitial) :
        ble(_ble), IDstr(ID_STR_CHARACTERISTIC_UUID, IDStrInitial, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY)
    {
        GattCharacteristic *charTable[] = {&IDstr};
        GattService         iDService(IDService::ID_SERVICE_UUID, charTable, sizeof(charTable) / sizeof(GattCharacteristic *));
        ble.gattServer().addService(iDService);
    }

    void sendIDStr(char* p_id_str) {
        ble.gattServer().write(IDstr.getValueHandle(), (uint8_t *)&p_id_str, 9 * sizeof(char));
    }

private:
    BLE                              &ble;
    ReadOnlyArrayGattCharacteristic<char, 9> IDstr;
};

#endif /* #ifndef __BLE_ID_SERVICE_H__ */