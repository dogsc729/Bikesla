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
#include <chrono>
#include <cstdint>
#include <ctime>
#include <events/mbed_events.h>

#include <mbed.h>
#include <string>
#include <vector>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "pretty_printer.h"

#include "LEDService.h"
#include "BuzzerService.h"
#include "MotionService.h"

#include "stm32l475e_iot01_accelero.h"


const static char DEVICE_NAME[] = "Bikesla T1";

static EventQueue event_queue(100 * EVENTS_EVENT_SIZE);

PwmOut pwmOut_led(PA_15);
PwmOut pwmOut_buzzer(PA_2);
AnalogIn aIn_hall(A0);

class Bikesla : ble::Gap::EventHandler {
public:
    Bikesla(BLE &ble, events::EventQueue &event_queue) :
        _ble(ble),
        _event_queue(event_queue),
        _led2(LED2, 0),
        _button(BLE_BUTTON_PIN_NAME, BLE_BUTTON_PIN_PULL),
        _led_service(NULL),
        _led_uuid(LEDService::SERVICE_UUID),
        _buzzer_service(NULL),
        _buzzer_uuid(BuzzerService::SERVICE_UUID),
        _motion_service(NULL),
        _motion_uuid(MotionService::SERVICE_UUID),
        _adv_data_builder(_adv_buffer)
        // _pDataXYZ(NULL),
        {
            _initialTime = clock();
            _t_start = clock();
            _hall_update = false;
            tmp_speeds.clear();
        }

    void start() {
        _ble.gap().setEventHandler(this);
        _ble.init(this, &Bikesla::on_init_complete);

        pwmOut_led.period_ms(LED_PWM_PERIOD);
        pwmOut_buzzer.period_ms(BUZZER_PWM_PERIOD);

        // _pDataXYZ = new int16_t[3] {0};
        // _event_queue.call_every(ACC_SAMPLE_T, this, &Bikesla::sync_acc_xyz);

        _event_queue.call_every(LED_CYCLE_MAX, this, &Bikesla::front_blink);
        
        _event_queue.call_every(SPEED_SAMPLE_T, this, &Bikesla::checkHall);

        _event_queue.dispatch_forever();
    }

private:
    void sync_led(void) {
        pwmOut_led.write(_led_service->getLightnessPercentage());
    }
    void sync_buzzer(void) {
        pwmOut_buzzer.write(_buzzer_service->getVolumePercentage());
    }
    void buzzer_ring(void) {
        pwmOut_buzzer.write(0.9);
    }
    void buzzer_mute(void) {
        pwmOut_buzzer.write(0);
    }
    void led_blink(int ms) {
        for (int i=0; i<ms; i+=500) {
            pwmOut_led.write(0.99);
            ThisThread::sleep_for(std::chrono::milliseconds(300));
            pwmOut_led.write(0);
            ThisThread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    void check_buzzer() {
        int32_t status = _buzzer_service->getStatus();
        if (status == 0) {
            pwmOut_buzzer.write(0);
        } else if (status == 1) {
            int t_on = _buzzer_service->getT_on();
            int t_off = _buzzer_service->getT_off();
            float duty = _buzzer_service->getVolumePercentage();

            int loop = BUZZER_CYCLE_MAX / _buzzer_service->getT_cycle();
            for (int i=0; i<loop; ++i) {
                pwmOut_buzzer.write(duty);
                ThisThread::sleep_for(std::chrono::milliseconds(t_on));
                pwmOut_buzzer.write(0);
                ThisThread::sleep_for(std::chrono::milliseconds(t_off));
            }
            pwmOut_buzzer.write(duty);
        } else if (status == 2) {
            buzzer_ring();
            ThisThread::sleep_for(std::chrono::milliseconds(2000));
            buzzer_mute();
        }
    }

    void et_try(void) {
        int32_t curr_speed = _motion_service->getSpeed();
        _motion_service->updateSpeed(curr_speed + 1);
    }


    void checkHall(void) {
        _hall_value = aIn_hall.read();
        // _event_queue.call(printf, "%lf\n", _hall_value);
        if (_hall_value < 0.77 | _hall_value > 0.83) {
            if (_hall_update) return;
            _hall_update = true;
            // _event_queue.call(printf, "Hall value %lf\n", _hall_value);
            clock_t t_end = clock();
            double t = double((t_end - _t_start)) / CLOCKS_PER_SEC;
            if (t > 0.01) {
                int32_t new_speed = int32_t(((CIRCUMFERENCE)/t) * 3600);
                // _event_queue.call(printf, "speed %d\n", new_speed);
                if (new_speed < 100) {
                    _motion_service->updateSpeed(new_speed);
                    if (_motion_service->locked()) {
                        if (_motion_service->unsafeMove()) {
                            buzzer_ring();
                            // _event_queue.call(_buzzer_service->updateState, 5021);
                        }
                    } else {
                        if (new_speed > 30) {
                            _buzzer_service->setState(8011);
                        }
                    }
                }
            }
            _t_start = clock();
        } else {
            _hall_update = false;
        }
    }

    // void sync_acc_xyz(void) {
    //     BSP_ACCELERO_AccGetXYZ(_pDataXYZ);
    // }

    /** Callback triggered when the ble initialization process has finished */
    void on_init_complete(BLE::InitializationCompleteCallbackContext *params) {
        if (params->error != BLE_ERROR_NONE) {
            printf("Ble initialization failed.");
            return;
        }

        print_mac_address();

        /* Setup primary service. */
        _led_service = new LEDService(_ble, 5000);
        _buzzer_service = new BuzzerService(_ble, 5000);
        _motion_service = new MotionService(_ble, 100, 0);

        _ble.gattServer().onDataWritten(this, &Bikesla::onDataWritten);

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
        _adv_data_builder.setLocalServiceList(mbed::make_Span(&_led_uuid, 1));
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
        _event_queue.call(printf, "LED: %d\n", _led_service->getState());
        _event_queue.call(printf, "Buzzer: %d\n", _buzzer_service->getState());
        _event_queue.call(printf, "Motion: %d\n", _motion_service->getState());
        _event_queue.call(printf, "speed: %d\n", _motion_service->getSpeed());
        _event_queue.call(this, &Bikesla::et_try);
        _led2 = !_led2;
        // if (_motion_service->locked()) {
        //     _motion_service->updateState(1);
        // } else {
        //     _motion_service->updateState(0);
        // }
    }

    void button_released(void) {
        return;
    }

    void front_blink(void) {
        int status = _led_service->getStatus();
        if (status == 0) {
            return;
        } else if (status == 1) {
            // if (_led_service->getLightness() > 0) {
            int t_on = _led_service->getT_on();
            int t_off = _led_service->getT_off();
            float duty = _led_service->getLightnessPercentage();

            int loop = LED_CYCLE_MAX / _led_service->getT_cycle();
            for (int i=0; i<loop; ++i) {
                pwmOut_led.write(duty);
                ThisThread::sleep_for(std::chrono::milliseconds(t_on));
                pwmOut_led.write(0);
                ThisThread::sleep_for(std::chrono::milliseconds(t_off));
            }
            pwmOut_led.write(duty);
            // }
        } else if (status == 2) {
            // find bike
            led_blink(2000);
        }
    }



private:
    /* Event handler */

    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&) {
        _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    }

    virtual void onDataWritten(const GattWriteCallbackParams *params) {
        // _event_queue.call(printf, "onDataWritten() !!\n");
        if ((params->handle == _motion_service->getValueHandle()) && (params->len >= 2)) {
            _event_queue.call(printf, "received motion value: %d\n", *(uint16_t*)(params->data));
            // _motion_service->setState(*(int32_t*)(params->data));
            _motion_service->updateState(*(uint16_t*)(params->data));
        } else if ((params->handle == _led_service->getValueHandle()) && (params->len >= 2)) {
            _event_queue.call(printf, "received LED value: %d\n", *(uint16_t*)(params->data));
            // _led_service->setState(*(int32_t*)(params->data));
            _led_service->updateState(*(uint16_t*)(params->data));
        } else if ((params->handle == _buzzer_service->getValueHandle()) && (params->len >= 2)) {
            _event_queue.call(printf, "received buzzer value: %d\n", *(uint16_t*)(params->data));
            _event_queue.call(this, &Bikesla::check_buzzer);
            // _buzzer_service->setState(*(int32_t*)(params->data));
            _buzzer_service->updateState(*(uint16_t*)(params->data));
        }
    }

private:
    BLE &_ble;
    events::EventQueue &_event_queue;

    clock_t _initialTime;

    DigitalOut  _led2;
    InterruptIn _button;

    LEDService *_led_service;
    UUID _led_uuid;

    BuzzerService *_buzzer_service;
    UUID _buzzer_uuid;

    MotionService *_motion_service;
    UUID _motion_uuid;

    std::vector<int> tmp_speeds;

    // int16_t *_pDataXYZ;

    clock_t _t_start;
    double _hall_value;
    bool _hall_update;

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

    Bikesla bikesla(ble, event_queue);
    bikesla.start();

    return 0;
}

