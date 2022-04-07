// HeNe laser exhibit.  2022
// Arduino IDE 1.8.19
// Arduino Boards 1.8.3
#include <AccelStepper.h>   // 1.61.0

// SOFTWARE CONFIG
const byte sleep_delay_s = 1;           // seconds to wait before putting stepper in low-current mode.
const byte exhibit_off_delay_m = 30;     // minutes to wait before turning off the exhibit
const int stepper_max_steps = 1424;     // number of steps controlled by the knob
const int stepper_reset_steps = -1500;  // number of steps to ensure mechanical stop is hit
const int stepper_speed = 2000;         // steps per second
const int stepper_reset_speed = 300;    // steps per second during reset (slower)
const int stepper_accel = 8000;         // steps per second^2
const int knob_samples = 100;           // number of knob samples to average
const int knob_deadband = 8;            // ADC threshold before exhibit is considered "awake"


// HARDWARE CONFIG
const float adc_max = 1011.0;           // Max value expected from the pot, ensures float division.
const byte pin_step = 4;                // stepper pulse pin
const byte pin_dir = 3;                 // stepper direction pin
const byte pin_knob = A0;               // knob analog read pin
const byte pin_motor_power = 7;         // motor enable pin (high = OFF)
const byte pin_laser_power = 8;         // laser enable pin (high = ON)
AccelStepper stepper(AccelStepper::DRIVER, pin_step, pin_dir);


// TRACKING VARIABLES
int knob_resolved_value = 0;            // averaged knob value
int knob_previous_value = 0;            // knob value for deadband comparison
int knob_buffer[knob_samples];          // knob samples to average
unsigned long knob_buffer_total = 0;    // sum of knob samples
byte knob_buffer_read_index = 0;        // index to read from buffer
bool power = 1;                         // exhibit laser power state
bool awake = 1;                         // exhibit motor power state
long last_touch_ms = 0;                 // time in milliseconds since last touch


// Sets the exhibit (laser) power state.
void set_power(bool state) {
    digitalWrite(pin_laser_power, state);
    power = state;
}


// Sets the exhibit (motor) sleep state.
void set_awake(bool state) {
    // enable pin pulled high turns off the motor.
    digitalWrite(pin_motor_power, !state);
    digitalWrite(LED_BUILTIN, state);
    awake = state;
}


// initial reset routine to position the motor from an unknown state.
void reset_stepper() {
    stepper.setAcceleration(stepper_accel);
    stepper.setMaxSpeed(stepper_reset_speed);
    
    // blocking call to reset the stepper position
    stepper.runToNewPosition(stepper_reset_steps);
    stepper.setCurrentPosition(0);
    
    // set normal running speed
    stepper.setMaxSpeed(stepper_speed);
}


// Arduino Setup routine
void setup() {
    memset(knob_buffer, 0, sizeof(knob_buffer));
    pinMode(pin_motor_power, OUTPUT);
    pinMode(pin_laser_power, OUTPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    set_power(1);
    set_awake(1);
    reset_stepper();
}


// Arduino loop routine
void loop() {
    sample_knob();
    update_state();
    if (power && awake) {update_position();}
}


// Updates the sleep and power state of the exhibit.
// Sleeps after no one has touched it in sleep_delay_s
// Powers down after no one has touched it in exhibit_off_delay_m
void update_state() {
    // compare current knob setting to last recorded value
    // and see if it's outside the deadband, if so, wake up
    if (abs(knob_resolved_value -  knob_previous_value) > knob_deadband) {
        
        last_touch_ms = millis();
        knob_previous_value = knob_resolved_value;

        if (!power) {
            set_power(1);
        }
        if (!awake) {
            set_awake(1);
        }
        return;
    }

    // Check the timer to see if it's time to sleep.
    if (awake) {
        if (millis() - last_touch_ms > sleep_delay_s * 1000) {
            set_awake(0);
        }
    }

    // Check the timer to see if it's time to shut down.
    if (power) {
        if (millis() - last_touch_ms > exhibit_off_delay_m * 60000) {
            set_power(0);
        }
    }
}


// Updates the motor position and run the motor.  Called frequently.
void update_position() {
    long position = (knob_resolved_value / adc_max) * stepper_max_steps;
    stepper.moveTo(position);
    stepper.run();
}

// Reads the knob's analog input and averages multiple samples
void sample_knob() {
    knob_buffer_total = knob_buffer_total - knob_buffer[knob_buffer_read_index];
    knob_buffer[knob_buffer_read_index] = analogRead(pin_knob);
    knob_buffer_total = knob_buffer_total + knob_buffer[knob_buffer_read_index];

    knob_buffer_read_index ++;

    // index wrapping
    if (knob_buffer_read_index >= knob_samples) {
        knob_buffer_read_index = 0;
    }
    
    knob_resolved_value = knob_buffer_total / knob_samples;
}
