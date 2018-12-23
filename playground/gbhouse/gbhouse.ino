#include <Adafruit_NeoPixel.h>
#include <Button.h>

// Button to toggle lights state OFF/SLOW/FAST
// Connect button between pin 3 and GND
Button button(3); 

// Which pin on the Arduino is connected to the NeoPixels?
#define PIN                   2

// Num neopixels attached to the Arduino 
#define NUMPIXELS             24


const int NUM_INTERVAL_OPTIONS = 3;
//-1 options means off
int updateIntervals[NUM_INTERVAL_OPTIONS] = {-1,50,1};
int updateIntervalIndex = 0; //loops first time

int tick = 0; //loops between 8-bit colors 0 - 255
bool lightsOn = false;

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pixels.begin();
  button.begin();

  while (!Serial) { };
  Serial.begin(9600);
  
  renderLightsOff();
}

void loop() {
  if (button.released()) {
    updateIntervalIndex++;
    if (updateIntervalIndex >= NUM_INTERVAL_OPTIONS) {
      updateIntervalIndex = 0;
      lightsOn = false;
      renderLightsOff();
    } else {
      lightsOn = true;
    }
  }
  
  if (lightsOn) {
    lightsRenderLoop();
    delay(updateIntervals[updateIntervalIndex]);
  }
}

void lightsRenderLoop() {
  if (tick >= 256) {
    tick = 0;
  }
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, Wheel((i*1+tick)));
  }
  pixels.show();
  tick++;
}

void renderLightsOff() {
  for(uint16_t i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  pixels.show();
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
    return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
