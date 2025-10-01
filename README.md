# Magic Dress Controller
Arduino controller built for Adafruit Feather BLE for the Magic Dress

This app works with the [Magic Dress Android App](https://github.com/keithfry/dress-android) firmware.

## Setup
### Install Adafruit boards
_You only need to do this once for the Arduino IDE_
1. In Arduino IE, open Settings / Preferences
2. Add the Adafruit board library URL in "Additional boards manager URL:"         
   https://adafruit.github.io/arduino-board-index/package_adafruit_index.json
3. Go to Tools > Board > Boards Manager.
4. Search for "feather" and find "Adafruit AVR Boards", click Install

### Install Adafruit Bluetooth Libraries
_You only need to do this once per project_
1. Open your IDE and open your Sketch
2. Access the Library Manager: Go to Sketch > Include Library > Manage Libraries....
3. Search for the Adafruit BluefruitLE nRF51 library and Adafruit Dotstar library, then install each

More on this library is here: https://learn.adafruit.com/adafruit-feather-32u4-bluefruit-le/installing-ble-library


## Runtime
### Configuration
1. NUM_PIXELS : number of pixels on the strip that will be illuminated, necessary for each sliding mode
2. PIXEL_PIN : status pin on the board that is used to display if the board is started, connected, etc. otherwise non-funcational.
3. DATA_PIN = 6, CLOCK_PIN = 5 : pins that the dotstar are connected to for data & clock information. 
> [!Important]
> If you've connected to different pins you'll need to change these values
 
### Initialization
setup() is called when the board is initialized, we do two steps:
1. Configures BLE
   1. create a device "FloraDress" (this is an old name when we were building it with a Flora)
   2. create a service with a registered callback for events we receive from a client
2. Setup the strip

### Client Requests
The client can request to change display modes

#### Modes
The following modes execute based on request from the client:
1. **shimmer()** : displays random variations of white at different intensities
2. **twinkle (n) ** : displays a random number of white segments up to n. Eg if n = 50, between 0-50 segments can be lit for a period of time 
3. **lightning()** : darkens the dress and randomly displays a single strand set of white for a short period of time, like lightning
4. **maizeAndBlue()** : Hail to the Victors!
5. **pulse (low, high)** : pulses a block of strand sets from low to high white for 1000ms
6. **mostlyWhite()** : sets the dress to all white with moderate-hight brightness. Looks great in reflections.
7. **dark()** : turns all the LEDs off, conserves battery.


   
