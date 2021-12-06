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
#include <mbed.h>

#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "pretty_printer.h"
#include "arm_math.h"
#include "math_helper.h"
#include "stm32l475e_iot01_accelero.h"

#include <vector>
#include <string>

#include "ButtonService.h"
#include "IDService.h"
#include "LEDService.h"

const static char DEVICE_NAME[] = "BIKESLA";

static EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);


class Bikesla : ble::Gap::EventHandler {
public:
    Bikesla(BLE &ble, events::EventQueue &event_queue) :
        _ble(ble),
        _event_queue(event_queue),
        _led1(LED1, 1),
        _led2(LED2, 0),
        _button(BLE_BUTTON_PIN_NAME, BLE_BUTTON_PIN_PULL),
        _button_service(NULL),
        _button_uuid(ButtonService::BUTTON_SERVICE_UUID),
        _id_service(NULL),
        _id_uuid(IDService::ID_SERVICE_UUID),
        _led_service(NULL),
        _led_uuid(LEDService::LED_SERVICE_UUID),
        _adv_data_builder(_adv_buffer) { }

    void start() {
        _ble.gap().setEventHandler(this);

        _ble.init(this, &Bikesla::on_init_complete);

        _event_queue.call_every(500, this, &Bikesla::blink);

        _event_queue.dispatch_forever();
    }

private:
    /** Callback triggered when the ble initialization process has finished */
    void on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
        if (params->error != BLE_ERROR_NONE) {
            printf("Ble initialization failed.");
            return;
        }

        print_mac_address();

        /* Setup primary service. */

        _button_service = new ButtonService(_ble, false /* initial value for button pressed */);

        char id_str[10] = "b07505026";
        _id_service = new IDService(_ble, id_str);

        _led_service = new LEDService(_ble, false);

        _ble.gattServer().onDataWritten(this, &Bikesla::on_data_written);

        _button.fall(Callback<void()>(this, &Bikesla::button_pressed));
        _button.rise(Callback<void()>(this, &Bikesla::button_released));

        start_advertising();
    }

    void start_advertising() {
        /* Create advertising parameters and payload */

        ble::AdvertisingParameters adv_parameters(
            ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
            ble::adv_interval_t(ble::millisecond_t(1000))
        );

        _adv_data_builder.setFlags();
        _adv_data_builder.setLocalServiceList(mbed::make_Span(&_button_uuid, 1));
        _adv_data_builder.setName(DEVICE_NAME);

        /* Setup advertising */

        ble_error_t error = _ble.gap().setAdvertisingParameters(
            ble::LEGACY_ADVERTISING_HANDLE,
            adv_parameters
        );

        if (error) {
            print_error(error, "_ble.gap().setAdvertisingParameters() failed");
            return;
        }

        error = _ble.gap().setAdvertisingPayload(
            ble::LEGACY_ADVERTISING_HANDLE,
            _adv_data_builder.getAdvertisingData()
        );

        if (error) {
            print_error(error, "_ble.gap().setAdvertisingPayload() failed");
            return;
        }

        /* Start advertising */

        error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

        if (error) {
            print_error(error, "_ble.gap().startAdvertising() failed");
            return;
        }
    }

    void button_pressed(void) {
        char id_str[10] = "b07505026";

        _event_queue.call(Callback<void(bool)>(_button_service, &ButtonService::updateButtonState), true);
        // _event_queue.call(Callback<void(char*)>(_id_service, &IDService::sendIDStr), id_str);
        if (_led2) {
            _event_queue.call(Callback<void(bool)>(_led_service, &LEDService::toggleLEDState), false);
        } else {
            _event_queue.call(Callback<void(bool)>(_led_service, &LEDService::toggleLEDState), true);
        }
        _led2 = !_led2;
    }

    void button_released(void) {
        _event_queue.call(Callback<void(bool)>(_button_service, &ButtonService::updateButtonState), false);
    }

    void on_data_written(const GattWriteCallbackParams *params) {
        if ((params->handle == _led_service->getValueHandle()) && (params->len == 1)) {
            _led2 = *(params->data);
        }
    }

    void blink(void) {
        _led1 = !_led1;
    }

private:
    /* Event handler */

    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&) {
        _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    }

private:
    BLE &_ble;
    events::EventQueue &_event_queue;

    DigitalOut  _led1;
    DigitalOut  _led2;
    InterruptIn _button;
    ButtonService *_button_service;
    IDService *_id_service;
    LEDService *_led_service;

    UUID _button_uuid;
    UUID _id_uuid;
    UUID _led_uuid;

    uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    ble::AdvertisingDataBuilder _adv_data_builder;
};

/** Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context) {
    event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

int main()
{
    BLE &ble = BLE::Instance();
    ble.onEventsToProcess(schedule_ble_events);

    Bikesla demo(ble, event_queue);
    demo.start();

    return 0;
}
