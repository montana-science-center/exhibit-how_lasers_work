#include <FastLED.h>        // 3.5.0
#include <AccelStepper.h>   // 1.61.0


// software config
const byte sleep_delay_s = 1;         // seconds to wait before putting stepper in low-current mode.
const byte laser_off_delay_m = 30;    // minutes to wait before turning off the laser
const int stepper_max_steps = 200;    // number of steps controled by the knob
const int stepper_reset_steps = -300; // number of steps to over-travel to ensure mechanical stop is hit
const int stepper_speed = 1000;       // steps per second
const int stepper_accel = 1000;       // steps per second^2
const int knob_sample_ms = 50;        // number of milliseconds betwene knob reads
const int knob_samples = 10;          // number of knob reads to average
const int knob_deadband = 8;          // ADC threshold before exhibit is considered "active"


// hardware config
const int adc_res = 1024;
const byte pin_step = 3;
const byte pin_dir = 4;
const byte pin_knob = A0;
const byte pin_motor_power = 7;
const byte pin_laser_power = 6;
AccelStepper stepper(AccelStepper::DRIVER, pin_step, pin_dir);


// tracking variables
int knob_resolved_value = 0;
int knob_sleep_value = 0;
int knob_buffer[knob_samples];
int knob_buffer_total = 0;
byte knob_buffer_read_index = 0;
bool power = 1;
bool awake = 1;
long last_touch_ms = 0;


void setup() {
    pinMode(pin_motor_power, OUTPUT);
    pinMode(pin_laser_power, OUTPUT);

    // configure stepper
    stepper.setMaxSpeed(stepper_speed);
    stepper.setAcceleration(stepper_accel);

    // init knob sample array to zero
    memset(knob_buffer, 0, sizeof(knob_buffer));

    digitalWrite(pin_motor_power, HIGH);
    digitalWrite(pin_laser_power, HIGH);

    reset_stepper();
}


void reset_stepper() {
    // blocking call to reset the stepper position
    stepper.runToNewPosition(stepper_reset_steps);
    stepper.setCurrentPosition(0);
}


void loop() {

    EVERY_N_MILLISECONDS(knob_sample_ms) {
        sample_knob();
    }

    if (power && awake) {
        update_position();
    }

    update_state();

}

void update_state() {

    if (abs(knob_resolved_value -  knob_sleep_value) > knob_deadband) {
        last_touch_ms = millis();
        knob_sleep_value = knob_resolved_value;

        if (!power) {
            digitalWrite(pin_laser_power, HIGH);
            power = 1;
        }
        
        if (!awake) {
            digitalWrite(pin_motor_power, HIGH);
            awake = 1;
        }
        
        return;
    }
    
    if (awake) {
        if (millis() - last_touch_ms > sleep_delay_s * 1000) {
            digitalWrite(pin_motor_power, LOW);
            awake = 0;
        }
    }

    if (power) {
        if (millis() - last_touch_ms > laser_off_delay_m * 60000) {
            digitalWrite(pin_laser_power, LOW);
            power = 0;
        }
    }
}


void update_position() {
    long position = (knob_resolved_value / adc_res) * stepper_max_steps;
    stepper.moveTo(position);
    stepper.run();
}


void sample_knob() {
    knob_buffer_total = knob_buffer_total - knob_buffer[knob_buffer_read_index];
    knob_buffer[knob_buffer_read_index] = analogRead(pin_knob);
    knob_buffer_total = knob_buffer_total + knob_buffer[knob_buffer_read_index];
    
    knob_buffer_read_index ++;

    if (knob_buffer_read_index >= knob_samples) {
        knob_buffer_read_index = 0;
    }
    
    knob_resolved_value = knob_buffer_total / knob_samples;
}
