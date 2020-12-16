#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "LedControl.h"

#define MUTE 0
#define SCROLL_TEXT "Ho Ho Ho! Merry X-mas! GOD JUL! "

// 8x8 LED
#define HARDWARE_TYPE MD_MAX72XX::GENERIC_HW
#define MAX_DEVICES 1
#define CLK_PIN     4
#define DATA_PIN    6
#define CS_PIN      5

// Ultrasonic sensor
#define TRIG_PIN                  9
#define ECHO_PIN                  8
#define DISTANCE_TO_ACTIVATE_CM   30
#define UPDATE_INTERVAL_MS        100

// Buzzer
#define BUZZER_PIN  2

// Light led
#define LIGHT_LED_PIN 11

// Number of idle snowflakes
#define NUM_SNOWFLAKES 6

/** LedControl for snowflakes
 pin 12 is connected to the DataIn 
 pin 11 is connected to the CLK 
 pin 10 is connected to LOAD 
 We have only a single MAX72XX.
 */
LedControl lc=LedControl(6,4,5,1);

// Jingle Bells!
int toneLengthMillis = 200;
int toneGapMillis = 80;

char notes[] = "eeeeeeegcde fffgfeeeeddedg";
int duration[] = {1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2};
int totNotes = sizeof(notes);

char emptyNote = ' ';
char notesName[] = { 'c', 'd', 'e', 'f', 'g' };
int tones[] = { 261, 293, 329, 349, 392 };

int currentNote = 0;
int lastNoteMillis = 0;
int nextNoteDelayMillis = 0;

// Idle state (snowflakes)
int snowflakes[NUM_SNOWFLAKES] = {-5,-1,-3,-6,-8,1};
int snowflakeCols[NUM_SNOWFLAKES] = {1,4,3,2,5,6};
int sensorUpdateIntervalMS = 200;
int lastSensorUpdateMS = 0;

// States
enum HouseState {
  BOOT,
  IDLE,
  PARTY
};
HouseState state = BOOT;

// LED helper
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  P.begin();
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LIGHT_LED_PIN, OUTPUT);
  
  Serial.begin(9600);

  delay(1000);

  changeState(IDLE);
}

void loop() {
  if (state == PARTY) {
    party();
  } else {
    idle();
  }
}

void changeState(HouseState nextState) {
  Serial.println("changeState()");
  if (nextState != state) {
    Serial.print("State changed:");
    Serial.println(state);

    if (state == PARTY) {
      P.displayClear();
      P.displayReset();
    }

    if (nextState == PARTY) {
      lastNoteMillis = 0;
      currentNote = 0;
      
      P.setIntensity(8);

    } else if (nextState == IDLE) {
      digitalWrite(LIGHT_LED_PIN, HIGH);
      noTone(BUZZER_PIN);
      
      P.setIntensity(1);
    }

    state = nextState;

    delay(100);
  }
}

void idle() {
  int currMillis = millis();

  if ((currMillis-lastSensorUpdateMS) >= sensorUpdateIntervalMS) {
    // The distance between sensor and obstacle (cm): 
    // distance = travel_distance / 2 = 0.034 × pulse_duration / 2 = 0.017 × pulse_duration
    // generate 10-microsecond pulse to TRIG pin
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);

    // measure duration of pulse from ECHO pin
    int duration_us = pulseIn(ECHO_PIN, HIGH);

    // calculate the distance
    int distance_cm = 0.017 * duration_us;

    // print the value to Serial Monitor
    //Serial.print("distance: ");
    //Serial.print(distance_cm);
    //Serial.println(" cm");

    if (duration_us>0 && distance_cm <= DISTANCE_TO_ACTIVATE_CM) {
      changeState(PARTY);
    } else {  
      updateSnowflakes();
    }
    lastSensorUpdateMS = millis();
  }
}

void party() {
  int currMillis = millis();
  
  if (P.displayAnimate()) {
    P.displayText(SCROLL_TEXT, PA_CENTER, 45/*P.getSpeed()*/, P.getPause(), PA_SCROLL_LEFT, PA_NO_EFFECT);
  }
    
  if ((currMillis-lastNoteMillis) >= nextNoteDelayMillis || currentNote == 0) {
    if (!MUTE) {
      playTone(notes[currentNote], duration[currentNote] * toneLengthMillis);
    }
    lastNoteMillis = currMillis;
    nextNoteDelayMillis = (duration[currentNote] * toneLengthMillis) + (duration[currentNote] * toneGapMillis);
    currentNote++;

    bool now = digitalRead(LIGHT_LED_PIN);
    digitalWrite(LIGHT_LED_PIN, !now);

    if (currentNote >= totNotes) {
      changeState(IDLE);
    }
  }
}

void updateSnowflakes() {
  lc.clearDisplay(0);

  for (int i=0; i<NUM_SNOWFLAKES; i++) {
    if (snowflakes[i] < 7) {
      snowflakes[i] = snowflakes[i] + 1;
      if (snowflakes[i] >= 0) {
        lc.setLed(0,snowflakeCols[i],snowflakes[i], true);
      }
    } else {
      snowflakes[i] = random(-10, -1);
      snowflakeCols[i] = random(0, 7);
    }
  }
}

void playTone(char note, int duration) {
  if (note == emptyNote) {
    noTone(BUZZER_PIN); 
  } else {
    for (int i = 0; i <= sizeof(tones); i++) {
      if (note == notesName[i]) {
        tone(BUZZER_PIN, tones[i], duration);
        break;
      }
    }
  }
}