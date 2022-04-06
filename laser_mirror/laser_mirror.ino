#include <AccelStepper.h>   // 1.61.0


// software config
const byte sleep_delay_s = 1;      // seconds to wait before putting stepper in low-current mode.
const byte laser_off_delay_m = 1;      // minutes to wait before turning off the laser
const int stepper_max_steps = 1424;    // number of steps controled by the knob
const int stepper_reset_steps = -1500; // number of steps to ensure mechanical stop is hit
const int stepper_speed = 2000;        // steps per second
const int stepper_reset_speed = 300;   // steps per second during reset
const int stepper_accel = 8000;        // steps per second^2
const int knob_sample_ms = 1;          // number of milliseconds betwene knob reads
const int knob_samples = 10;           // number of knob reads to average
const int knob_deadband = 8;           // ADC threshold before exhibit is considered "active"


// hardware config
const float adc_res = 1011.0; // ensure float division
const byte pin_step = 3;
const byte pin_dir = 4;
const byte pin_knob = A0;
const byte pin_motor_power = 7;
const byte pin_laser_power = 8;
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


void set_power(bool state) {
    digitalWrite(pin_laser_power, state);
    power = state;
}


void set_awake(bool state) {
    // enable pin pulled high turns off the motor.
    digitalWrite(pin_motor_power, !state);
    awake = state;
}


void reset_stepper() {
    stepper.setAcceleration(stepper_accel);
    
    // set reset speed
    stepper.setMaxSpeed(stepper_reset_speed);
    
    // blocking call to reset the stepper position
    stepper.runToNewPosition(stepper_reset_steps);
    stepper.setCurrentPosition(0);
    
    // set normal running speed
    stepper.setMaxSpeed(stepper_speed);
}


void setup() {

    Serial.begin(9600);

    // init knob sample array to zero
    memset(knob_buffer, 0, sizeof(knob_buffer));

    pinMode(pin_motor_power, OUTPUT);
    pinMode(pin_laser_power, OUTPUT);
    set_power(1);
    set_awake(1);

    // configure stepper
    reset_stepper();
}


void loop() {

    sample_knob();
    //Serial.println(knob_resolved_value);

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
            set_power(1);
        }
        if (!awake) {
            set_awake(1);
        }
        return;
    }

    if (awake) {
        if (millis() - last_touch_ms > sleep_delay_s * 1000) {
            set_awake(0);
        }
    }

    if (power) {
        if (millis() - last_touch_ms > laser_off_delay_m * 60000) {
            set_power(0);
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
