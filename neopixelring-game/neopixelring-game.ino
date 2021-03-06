// "Shoot" lights to the neopixel ring. If the light position is already occupied => GAME OVER!
// Potential variants:
//   1. Allow light to wander in both directions.
#include <Adafruit_NeoPixel.h>
#include <JC_Button.h>

// Pin connected to the action button
#define BUTTON_PIN    3
// Pin connected to NeoPixel ring
#define NEOPIXEL_PIN  2
// Num NeoPixels in ring 
#define NUM_PIXELS    24
  
// Button with 10ms debounce
Button button(BUTTON_PIN, 10);
// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Game/Application states, mapped to some functions
typedef enum {STATE_IDLE = 0, STATE_PLAYING = 1, STATE_GAMEOVER = 2} GameState;
GameState gameState = -1;
void (*stateTable[])() = {idleLoop, gameLoop, gameOverLoop};
void (*lightRenderStateTable[])() {idlingLights, playingLights, gameOverLights};

static const int START_PIXEL_INDEX = 0;
static const int START_LEVEL_UPDATE_INTERVAL = 250;
  
static long currentMS = 0;
static long lastUpdateMS = 0;
static int updateIntervalMS = START_LEVEL_UPDATE_INTERVAL;

static int numActiveLights = 0;
static int activeLights[24] = {};

static int currentLevel = 0;

static int collectedLights = 0;

void setup() {
  pixels.begin();
  button.begin();

  while (!Serial) { };
  Serial.begin(19200);

  enterState(STATE_IDLE);
}

bool enterState(GameState nextState) {
  if (gameState == nextState) {
    return false;
  }
  
  if (nextState == STATE_IDLE) {
    updateIntervalMS = START_LEVEL_UPDATE_INTERVAL;
    clearLights(true);
    numActiveLights = 0;
  } else if (nextState == STATE_PLAYING){
    // Start game with first shot
    currentLevel = 1;
    clearLights(true);
    numActiveLights = 0;
    fireLight(); 
  } else if (nextState == STATE_GAMEOVER){
    collectedLights = 0;
    Serial.println("GAME OVER");
    Serial.println(numActiveLights);
  }

  gameState = nextState;  
  return true;
}

void loop() {
  button.read();
  stateTable[gameState]();
}

void idleLoop() {
  if (button.wasPressed()) {
    enterState(STATE_PLAYING);
  }
  
  lightsLoop();
}

void gameOverLoop() {
  if (button.wasPressed()) {
    enterState(STATE_IDLE);
  }
  
  lightsLoop();
}

void gameLoop() {
  if (button.wasPressed()) {
    bool fired = fireLight();
    
    if (!fired) {
      // Not able to fire -> game over!
      enterState(STATE_GAMEOVER);
    } else {
      Serial.println("NICE SHOT!");
      Serial.println(numActiveLights);
      levelUp();
    }
  }
  
  lightsLoop();
}

void levelUp() {
  currentLevel++;

  // Increase speed every third level (if possible)
  if (currentLevel%3 == 0 && updateIntervalMS > 25) {
    Serial.println("Speed increase!!");
    updateIntervalMS -= 25;
  }
}

void lightsLoop() {
  currentMS = millis();
  
  if ((currentMS - lastUpdateMS) >= updateIntervalMS) {
    clearLights();
    lightRenderStateTable[gameState]();  
    pixels.show();
    lastUpdateMS = currentMS;
  }
}

void idlingLights() {
  pixels.setPixelColor(random(23), pixels.Color(random(255),random(255),random(255)));
}

void playingLights() {
  for (int i=0; i<numActiveLights; i++) {
    activeLights[i] += 1;
    //wrap
    if (activeLights[i] >= NUM_PIXELS) {
      activeLights[i] = 0;
    }
    pixels.setPixelColor(activeLights[i], pixels.Color(0,255,0));
  }
}

// Stack up all lights so its easy to count them. And make em dead red!
void gameOverLights() {
  for (int i=0; i<numActiveLights; i++) {
    if (activeLights[i] < NUM_PIXELS-collectedLights-1) {
      activeLights[i] += 1;
      //wrap
      if (activeLights[i] >= NUM_PIXELS) {
        activeLights[i] = 0;
      }
    }
    if (activeLights[i] == NUM_PIXELS-collectedLights-1) {
      collectedLights++;
    }
    pixels.setPixelColor(activeLights[i], pixels.Color(255,0,0));
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
