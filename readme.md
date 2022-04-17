
# HeNe laser notes


## Control

### Settings
Currently set for:
* 1-second before motor sleep
* 10-minutes before laser power off

Laser and motor will turn on when the knob is rotated out of its deadband (in ADC units).

### Resetting
The electronics can be re-set by cycling power.  Alternatively, the reset button can be pressed on the arduino.

The motor's position is reset by mechanically hitting a stop.

This can happen in two conditions.
1. Upon arduino reset, a subroutine will deliberately over-travel the motor so it resets against the stop.
2. Upon enough missed steps, the motor with hit the end stop and reset.

### Calibration
To calibrate the mirror's position:
1. Loosen the two set screws on the motor pinion pulley.
2. Remove the pinion and belt.
3. Adjust the mirror micrometer screws so the laser is actively lasing.
4. Manually rotate the motor shaft so the flat is facing the large pulley.
5. Re-assemble the belt and pinion, moving the large pulley as little as possible, but ensuring the teeth are engaged.
6. Gently tighten the pinion set screws, ensuing the long screw engages with the flat on the motor shaft.

The procedure will place the optimal laser alignment position roughly in the middle of motor travel.  It can however be positioned anywhere desired within the range.

### Maintenance
No regular maintenance is expected, but here are some things to look for:
* If the belt is loose, there are slots mounting the motor allowing it to be tensioned more.


### Components
* [Arduino Nano](https://store-usa.arduino.cc/products/arduino-nano/)
* [EasyDriver v4.4](https://www.schmalzhaus.com/EasyDriver/)
* [Thor Labs KM100 mount ](https://www.thorlabs.com/thorproduct.cfm?partnumber=KM100#ad-image-0)
* [STEPPERONLINE 8HS11-0204S](https://www.omc-stepperonline.com/nema-8-stepper-motor/nema-8-bipolar-1-8deg-1-6ncm-2-3oz-in-0-2a-4-8v-20x20x28mm-4-wires-8hs11-0204s)
* [McMaster 1375K52 Timing Pulley, 42T](https://www.mcmaster.com/1375K52/)
* [McMaster 1375K14 Timing Pulley, 14T](https://www.mcmaster.com/1375K14/)
* [McMaster 1679K71 MXL Series, 1/8" Width, Trade No. 56mxl012](https://www.mcmaster.com/1679K71/)
* Custom mounting brackets
* Custom pcb (re-use from another project)

### IO:
#### Power
6-12vdc center+, 9v preferred.  Motor current is set based on 9v.

#### Laser control
**Orange:** 0-5vdc ttl
**White:** Ground

#### Potentiometer
This should be replaced with a slip-clutch pot at some point.
**value:** 100k (5-100 is fine)
**Orange:** +5vcd
**Blue:** Ground
**White:** Signal

#### Motor
**Black:** A+
**Green:** A-
**Red:** B+
**Blue:** B-
**Bare** Shield


## Software dependencies
This code requires the [AccelStepper](https://www.arduino.cc/reference/en/libraries/accelstepper/) library. This can be installed using Arduino's library manager.
