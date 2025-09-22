#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#if not defined (_VARIANT_ARDUINO_DUE_X_) && not defined (_VARIANT_ARDUINO_ZERO_)
#include <SoftwareSerial.h>
#endif


#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "Adafruit_BLEGatt.h"
#include "BluefruitConfig.h"

#include <Adafruit_NeoPixel.h>

#include <Adafruit_DotStar.h>
#define NUMPIXELS 60 // Number of LEDs in strip

#ifdef __AVR__
  #include <avr/power.h>
#endif

#define FACTORYRESET_ENABLE        1
#define MINIMUM_FIRMWARE_VERSION   "0.7.0"

#define BLUEFRUIT_HWSERIAL_NAME      Serial1


//
// To use hardware serial (preferred)
// FLORA:
//Adafruit_BluefruitLE_UART ble(BLUEFRUIT_HWSERIAL_NAME, BLUEFRUIT_UART_MODE_PIN);

// FEATHER
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);

Adafruit_BLEGatt gatt =   Adafruit_BLEGatt(ble);


// Indicator
#define PIXEL_PIN 8
//Adafruit_NeoPixel light = Adafruit_NeoPixel(1, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
#define NUM_COLORS 6
uint32_t colors[NUM_COLORS];
int colorIdx=0;

// Green line
#define DATAPIN    6
// Yellow line
#define CLOCKPIN   5
Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);


int32_t charid_number;

void pc(int idx, uint32_t color) {
  // Offset for indicator
  idx = idx+1;
  // Range limit
  if (idx >= NUMPIXELS) return;
  strip.setPixelColor(idx, color);
}

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void connected(void)
{
  Serial.println( F("Connected") );
//  light.setPixelColor(0, colors[1]); // Green
//  light.show();
}

void disconnected(void)
{
  Serial.println( F("Disconnected") );
//  light.setPixelColor(0, colors[0]); // Red
//  light.show();
}

// Timestamp for the pulse(...) option
unsigned long pulseTS=0;

// Start with shimmer I guess.
byte lightVal=0;
byte oldLightVal=0;

void BleGattRX(int32_t chars_id, uint8_t data[], uint16_t len)
{
  Serial.print( F("[BLE GATT RX] (" ) );
  Serial.print(chars_id);
  Serial.print(") ");
  oldLightVal = lightVal;
  lightVal = data[0];
  Serial.println(lightVal);

  // Reset the pulse on every received display change
  pulseTS=millis();

  // And clear the contents of the strip.
  if (lightVal != oldLightVal) {
    strip.clear();
  }

  // Turn it back on once if we go from dark to light.
  if (lightVal != 20) strip.setBrightness(255);


  Serial.println("BleGattRX");
  colorIdx++;
  if (colorIdx >= NUM_COLORS) colorIdx=0;
}

uint8_t service_id;
unsigned long ts=0;

void setup() {
//  colors[0]=light.Color(255,0,0);
//  colors[1]=light.Color(0,255,0);
//  colors[2]=light.Color(0,0,255);
//  colors[3]=light.Color(255,0,255);
//  colors[4]=light.Color(0,255,255);
//  colors[5]=light.Color(255,255,0);

//  light.begin();
//  light.show();  delay(200);

  //WHITE
//  light.setPixelColor(0, light.Color(255,255,255));
//  light.show();
  
  // This doesn't work when the serial port is not connected
  // so in order to get it run on it's own, comment this line out
  //while (!Serial);  // required for Flora & Micro
  delay(500);

  
  // RED -- Connect to serial
//  light.setPixelColor(0, colors[0]);
//  light.show();
  Serial.begin(115200);

  // GREEN -- ready to connect bluetooth
//  light.setPixelColor(0, colors[1]);
//  light.show();


  /* Initialise the module */
  Serial.print(F("Initialising the Bluefruit LE module: "));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  Serial.println( F("OK!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
  }

  if ( !ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) )
  {
    error( F("Callback requires at least 0.7.0") );
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  // Needed for Flora
  //ble.setInterCharWriteDelay(5); // 5 ms

  if (!ble.sendCommandCheckOK("AT+GAPDEVNAME=FloraDress")) {
    Serial.println("Couldn't change device name");
  }

  // Loop a few times trying to connect.
  int loopCount=20;
  service_id = 0;
  while (service_id == 0 && (loopCount-- > 0)) {
    service_id = gatt.addService(0x1888);
    Serial.print("service_id: "); Serial.println(service_id);
    delay(200);
  }

  // Loop a few times trying to connect.
  charid_number = 0;
  loopCount=20;
  while (charid_number == 0 && (loopCount-- > 0)) {
    charid_number = gatt.addCharacteristic(0x1777, GATT_CHARS_PROPERTIES_READ | GATT_CHARS_PROPERTIES_NOTIFY | GATT_CHARS_PROPERTIES_BROADCAST | GATT_CHARS_PROPERTIES_WRITE, 1, 1, BLE_DATATYPE_INTEGER);
    Serial.print("characteristic_id: "); Serial.println(charid_number);
    delay(500);
  }

  // Advertise the service 0x88, 0x18
  uint8_t advdata[] { 0x02, 0x01, 0x06, 0x03, 0x02, 0x88, 0x18 /*, 0x0a, 0x18 */};
  loopCount=20;
  while (!ble.setAdvData( advdata, sizeof(advdata)) && (loopCount-- > 0)) {
    delay(500);
  }

  Serial.println("reset()");
  ble.reset();
  delay(200);

  /* Set callbacks */
  Serial.println("Setting callbacks");
  ble.setConnectCallback(connected);
  ble.setDisconnectCallback(disconnected);


  // We do it multiple times because sometimes the first time gives an error.
  Serial.println("Setting GATT callback ");
  delay(200);
  ble.setBleGattRxCallback(charid_number, BleGattRX);
  delay(200);
  ble.setBleGattRxCallback(charid_number, BleGattRX);


  // BLUE - bluetooth started, ready to connect.
//  light.setPixelColor(0, colors[2]);
//  light.show();

  strip.begin(); // Initialize pins for output
  strip.clear();
  strip.setBrightness(255);
  strip.show();  // Turn all LEDs off ASAP
  randomSeed(10);

  ts = millis();
}

void twinkle(int num) {
  strip.setBrightness(255);
  for (int i=0; i < NUMPIXELS; i++) {
    if (random(1, 100) < num) {
      pc(i, 0xffffff);
    }
    else {
      pc(i, 0x000000);
    }
  }
  strip.show();
  delay(120);
}

int lgtCount=0;
int lgtBright=255;
int lgtSeg=0;
int NUMSEGS=5;
int NUMPIX_PER_SEGS=(NUMPIXELS/NUMSEGS);
void lightning() {
  // Start over
  if (lgtCount==0) {
      strip.clear();
      strip.setBrightness(0);
      strip.show();
      delay(random(150,800));
      lgtCount = random(5,30);
      lgtBright = 255;
      lgtSeg = random(0,NUMSEGS);
  }
  int low = (lgtSeg*NUMPIX_PER_SEGS);
  int high = ((lgtSeg+1)*NUMPIX_PER_SEGS);
  if (low < 0) low=0;
  if (high >= NUMPIXELS) high=NUMPIXELS;
  for (int i=low; i < high; i++) {
    pc(i, 0xffffff);
  }
  strip.setBrightness(lgtBright);
  strip.show();
  
  lgtCount--;
  lgtBright = (int)(0.75f*(float)(lgtBright));
  delay(15); // was 15s
}
uint32_t rbColors[] = {
  0x00ff00, // red
  0x66ff00, // orange 
  0xdaff21, // yellow
  0xff0000, // green 
  0x0000ff, // blue
  0x002266, // indigo
  0x00ffff // violet
};


int srbIdx=0;
int srbBright=0;
boolean srbDecrease=true;
void shiftingRainbow(int delayVal, boolean pulse) {
  for (int i=0; i < NUMPIXELS; i++) {
      int v = i/8;
      int idx=(srbIdx+i)%NUMPIXELS;
      if (v >6) v=6;
      pc(idx, rbColors[v]);
  }
  strip.show();
  if (pulse) {
    strip.setBrightness(255-(srbBright*11));
  }
  else {
    strip.setBrightness(255);
  }
  delay(delayVal);
  srbIdx++;
  if (srbIdx >= NUMPIXELS) srbIdx=0;

  if (srbDecrease) {
    srbBright++;
    if (srbBright > 18) {
      srbDecrease = false;
    }
  } else {
    srbBright--;
    if (srbBright <=0) {
      srbDecrease=true;
    }
  }
}

void maizeAndBlue() {
  srbIdx++;
  if (srbIdx >= NUMPIXELS) srbIdx=0;
  for (unsigned int idx=0; idx < NUMPIXELS; idx++) {
    // 10 blue
    unsigned int colIdx = idx+srbIdx;
    if ((colIdx%20)<10) {
      pc(idx,0x0000ff);
    }
    else {
      pc(idx,0xdaff21);
    }
  }
  strip.show();
  delay(80);
}

int mnbFadeIdx=0;
boolean mnbFadeShowBlue=false;
void maizeNBlueFade() {
}


uint32_t WS_COLORS [] = {
  0xffffff,
  0x555555,
  0x111111,
  0x070707,
  0x030303,
  0x020202,
  0x010101
};
#define WS_LEN sizeof(WS_COLORS)/sizeof(uint32_t)
int wsIdx=0;

void whiteSlide() {
  for (int i=0; i < NUMPIXELS; i++) {
    pc(i, WS_COLORS[(wsIdx+i)%WS_LEN]);
  }
  strip.show();
  wsIdx++;
  delay(50);
}


unsigned short SHM_COLORS_IDX[NUMPIXELS];
uint32_t SHM_COLORS[]={
  0xffffff,
  0x555555,
  0x111111,
  0x070707,
  0x030303,
  0x020202,
  0x010101,
  0x020202,
  0x030303,
  0x070707,
  0x111111,
  0x555555,
  0xffffff
};
#define  SHM_COLORS_LEN sizeof(SHM_COLORS)/sizeof(uint32_t)
bool shmInit=false;

void shimmer() {
  // Randomize first time.
  if (!shmInit) {
    shmInit=true;
    for (int i=0; i < NUMPIXELS; i++) {
      SHM_COLORS_IDX[i] = random(0,SHM_COLORS_LEN-1);
    }
//    for (int i=0; i < WS_LEN; i++) {
//      SHM_COLORS[i] = SHM_COLORS[WS_LEN-i-1] = WS_COLORS[i];
//    }
  }
  else {
    for (int i=0; i < NUMPIXELS; i++) {
      // Set the color
      unsigned short idx = SHM_COLORS_IDX[i];
      pc(i, SHM_COLORS[idx%SHM_COLORS_LEN]);
      SHM_COLORS_IDX[i] = idx+1;
    }
    strip.show();
  }
  delay(50);
}


void dark() {
  strip.clear();
  strip.setBrightness(0);
  strip.show();
  delay(100);
}

void pulse(int low, int high) {
  unsigned long diff = millis()-pulseTS;
  if (diff < 1000) {
    for (int i=low; i < high; i++) {
      pc(i, 0xffffff); // WHITE
    }
    int newB = 255-((255*diff)/200);
    if (newB<0) newB=0;
    strip.setBrightness(newB);
    strip.show();
  }
  else {
    dark();
  }
  delay(20);
}

void mostlyWhite() {
  for (int i=0; i < NUMPIXELS; i++) {
    pc(i, 0xffffff);
  }
  strip.setBrightness(180);
  strip.show();
  delay(20);
}

void loop() {
  // put your main code here, to run repeatedly:
  ble.update(20);

  switch (lightVal) {
    case 0:
      shimmer();
      //whiteSlide();
    break;
    case 1:
      twinkle(50);
    break;
    case 2:
      lightning();
    break;
    case 3:
      shiftingRainbow(40, true);
    break;
    case 4:
      whiteSlide();
    break;
    case 5:
      twinkle(20);
    break;
    case 6:
      shiftingRainbow(40, false);
    break;
    case 7:
      maizeAndBlue();
    break;
    case 8:
      // Pulse ALL
      pulse(0,NUMPIXELS);
    break;
    case 9:
      // PULSE RIGHT
      pulse(0,NUMPIXELS/2);
    break;
    case 10:
      // PULSE LEFT
      pulse((NUMPIXELS/2)+1,NUMPIXELS);
    break;
    case 11:
      mostlyWhite();
    break;
    case 20:
    default:
      dark();
    break;
  }

}
