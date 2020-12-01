#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

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

// States
enum HouseState {
  IDLE,
  PARTY
};
HouseState state = IDLE;

// LED helper
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

void setup() {
  P.begin();
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  if (state == PARTY) {
    play();
  } else {
    idle();
  }
}

void changeState(HouseState nextState) {
  Serial.println("changeState()");
  if (nextState != state) {
    state = nextState;
    Serial.print("State changed:");
    Serial.println(state);

    if (state == PARTY) {
      lastNoteMillis = 0;
      currentNote = 0;
    } else if (state == IDLE) {
      noTone(BUZZER_PIN);
        
      P.displayClear();
      P.displayReset();
    }
  }
}

void idle() {
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
  Serial.print("distance: ");
  Serial.print(distance_cm);
  Serial.println(" cm");

  if (duration_us>0 && distance_cm <= DISTANCE_TO_ACTIVATE_CM) {
    changeState(PARTY);
  } else {  
    delay(UPDATE_INTERVAL_MS);
  }
}

void play() {
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

    if (currentNote >= totNotes) {
      changeState(IDLE);
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