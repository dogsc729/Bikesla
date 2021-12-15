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
#include <cstdint>
#include <events/mbed_events.h>

#include <mbed.h>
#include <string>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "pretty_printer.h"

#include "LEDService.h"
#include "LED.h"


#include "ButtonService.h"


const static char DEVICE_NAME[] = "FuckYouDontConnect";

static EventQueue event_queue(/* event count */ 10 * EVENTS_EVENT_SIZE);

int32_t led_state_init = 2400;
int32_t led_state_try = 3311;
PwmOut pwmOut_front_led(PA_15);

class BatteryDemo : ble::Gap::EventHandler {
public:
    BatteryDemo(BLE &ble, events::EventQueue &event_queue) :
        _ble(ble),
        _event_queue(event_queue),
        _led1(LED1, 1),
        _led2(LED2, 0),
        _button(BLE_BUTTON_PIN_NAME, BLE_BUTTON_PIN_PULL),
        _button_service(NULL),
        _button_uuid(ButtonService::BUTTON_SERVICE_UUID),
        _led_service(NULL),
        _led_uuid(LEDService::LED_SERVICE_UUID),
        _front_led(NULL),
        _adv_data_builder(_adv_buffer) { }

    void start() {
        _front_led = new LED();

        _ble.gap().setEventHandler(this);

        _ble.init(this, &BatteryDemo::on_init_complete);

        _event_queue.call_every(500, this, &BatteryDemo::blink);

        _event_queue.dispatch_forever();
    }

private:
    void sync_front_led(void) {
        pwmOut_front_led.period_ms(_front_led->get_pwm_period());
        pwmOut_front_led.write(_front_led->get_pwm_duty_s());
    }

    /** Callback triggered when the ble initialization process has finished */
    void on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
        if (params->error != BLE_ERROR_NONE) {
            printf("Ble initialization failed.");
            return;
        }

        print_mac_address();

        /* Setup primary service. */

        _button_service = new ButtonService(_ble, false);

        _led_service = new LEDService(_ble, led_state_init);

        _ble.gattServer().onDataWritten(this, &BatteryDemo::onDataWritten);

        _button.fall(Callback<void()>(this, &BatteryDemo::button_pressed));
        _button.rise(Callback<void()>(this, &BatteryDemo::button_released));

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
        _event_queue.call(Callback<void(bool)>(_button_service, &ButtonService::updateButtonState), true);

        // _event_queue.call(Callback<void(int32_t)>(_led_service, &LEDService::updateLEDState), led_state_try);
        // _front_led->next_flash_mode();
        _event_queue.call(printf, "status: %d, flash_mode: %d\n", _front_led->get_status(), _front_led->get_flash_mode());
        _event_queue.call(printf, "pwm_duty: %d, pwm_period: %d\n", _front_led->get_pwm_duty(), _front_led->get_pwm_period());

        _led2 = !_led2;
    }

    void button_released(void) {
        _event_queue.call(Callback<void(bool)>(_button_service, &ButtonService::updateButtonState), false);
    }


    void blink(void) {
        _led1 = !_led1;
    }

    // void front_blink(void) {
    //     if (_front_led->get_status() && _front_led->get_pwm_duty() > 0) {
    //         int sleep_time = _front_led->get_cycle() - _front_led->get_t_on();
    //         if (sleep_time > 0) {
    //             int loop = LED_CYCLE_MAX / _front_led->get_cycle();
    //             sync_front_led();

    //             for (int i=0; i<loop; ++i) {
    //                 ThisThread::sleep_for(std::chrono::milliseconds( _front_led->get_t_on()));
    //                 _front_led->turn_off();
    //                 ThisThread::sleep_for(std::chrono::milliseconds(sleep_time));
    //                 _front_led->turn_on();
    //             }
    //         }
    //     }
    // }

private:
    /* Event handler */

    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&) {
        _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    }

    virtual void onDataWritten(const GattWriteCallbackParams *params) {
        _event_queue.call(printf, "onDataWritten() !!\n");
        if ((params->handle == _led_service->getValueHandle()) && (params->len >= 2)) {
            _event_queue.call(printf, "received: %d\n", *(uint16_t*)(params->data));
            _front_led->ble_set_int(*(uint16_t*)(params->data));
        }
    }

private:
    BLE &_ble;
    events::EventQueue &_event_queue;

    DigitalOut  _led1;
    DigitalOut  _led2;
    InterruptIn _button;
    ButtonService *_button_service;
    LEDService *_led_service;
    LED *_front_led;

    UUID _button_uuid;
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

    BatteryDemo demo(ble, event_queue);
    demo.start();

    return 0;
}

