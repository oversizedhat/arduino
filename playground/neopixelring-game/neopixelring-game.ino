// "Shoot" lights to the neopixel ring. If the light position is already occupied => GAME OVER!
// Potential variants:
//   1. Allow light to wander in both directions.

#include <Adafruit_NeoPixel.h>
#include <Button.h>

// Connect button between pin 3 and GND
Button button(3); 
  
// Which pin on the Arduino is connected to the NeoPixels?
#define PIN         2
  
// Num neopixels attached to the Arduino 
#define NUM_PIXELS  24
  
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
  
static const int START_PIXEL_INDEX = 0;
static const int START_LEVEL_UPDATE_INTERVAL = 250;
  
static long currentMS = 0;
static long lastUpdateMS = 0;
static int updateIntervalMS = START_LEVEL_UPDATE_INTERVAL;

static int numActiveLights = 0;
static int activeLights[24] = {};

static const int GAME_STATE_PLAYING = 10;
static const int GAME_STATE_DYING   = 11;
static const int GAME_STATE_IDLE    = 12;

static int currentLevel = 0;
static int gameState = 0;// NOT SET

void setup() {
  pixels.begin();
  button.begin();

  while (!Serial) { };
  Serial.begin(19200);

  enterState(GAME_STATE_IDLE);
}

bool enterState(int state) {
  if (gameState == state) {
    return false;
  }
  
  if (state == GAME_STATE_IDLE) {
    updateIntervalMS = START_LEVEL_UPDATE_INTERVAL;
    clearLights(true);
    numActiveLights = 0;
  } else if (state == GAME_STATE_PLAYING){
    // Start game with first shot
    currentLevel = 1;
    clearLights(true);
    numActiveLights = 0;
    fireLight(); 
  } else if (state == GAME_STATE_DYING){
    Serial.println("GAME OVER");
    Serial.println(numActiveLights);
  }

  gameState = state;  
  return true;
}

void loop() {
  if (gameState == GAME_STATE_IDLE) {
    idleLoop();
  } else if (gameState == GAME_STATE_PLAYING) {
    gameLoop();
  } else if (gameState == GAME_STATE_DYING) {
    dyingLoop();
  }
}

void idleLoop() {
  if (button.pressed()) {
    enterState(GAME_STATE_PLAYING);
  }
}

void dyingLoop() {
  if (button.pressed()) {
    enterState(GAME_STATE_IDLE);
  }
  
  lightsGameLoop(pixels.Color(255,0,0));
}

void gameLoop() {
  if (button.pressed()) {
    bool fired = fireLight();
    
    if (!fired) {
      // Not able to fire -> game over!
      enterState(GAME_STATE_DYING);
    } else {
      Serial.println("NICE SHOT!");
      Serial.println(numActiveLights);
      levelUp();
    }
  }
  
  lightsGameLoop(pixels.Color(0,255,0));
}

void lightsGameLoop(uint32_t lightColor) {
  currentMS = millis();

  if ((currentMS - lastUpdateMS) >= updateIntervalMS) {
    clearLights();
    
    for (int i=0; i<numActiveLights; i++) {
      activeLights[i] += 1;
      if (activeLights[i] >= NUM_PIXELS) {
        activeLights[i] = 0;
      }
      pixels.setPixelColor(activeLights[i], lightColor);
    }
    
    pixels.show();
    lastUpdateMS = currentMS; 
  }
}

bool fireLight() {
  // Check if someone is already occuping the entry pixel,
  // if that is the case, we cannot shoot
  for (int i=0; i<numActiveLights; i++) {
    if (activeLights[i] == START_PIXEL_INDEX) {
      return false;
    }
  }
  // Add a new light
  activeLights[numActiveLights] = START_PIXEL_INDEX;
  numActiveLights++;
  return true;
}

void clearLights() {
  clearLights(false);
}

void clearLights(bool render) {
  for(uint16_t i=0; i<NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0,0,0));
  }
  if (render)
    pixels.show();
}

void levelUp() {
  currentLevel++;

  // Increase speed every third level (if possible)
  if (currentLevel%3 == 0 && updateIntervalMS > 25) {
    Serial.println("Speed increase!!");
    updateIntervalMS -= 25;
  }
}