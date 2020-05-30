/**
 * A little math game using a small LED screen and two buttons. 
 * 
 * As a equation is presented in the LED screen, players have 4 seconds to click if the equation is correct.
 * If the player is correct 1 point is awarded, if not correct 2 points are deducted.
 * 
 */

#include <JC_Button.h>
#include <U8glib.h>
//#include <U8g2lib.h>

// Buttons with 10ms debounce
Button playerOne(2, 10);
Button playerTwo(13, 10);

 // LED
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
U8GLIB_SSD1306_128X64 oled(U8G_I2C_OPT_NONE);

static const int WIN_SCORE           = 3;
static const int QUESTION_TIMEOUT_MS = 4000;

static long currentMs             = 0;
static long lastQuestionUpdateMs  = 0;

int questionIndex = 0;

int playerScores[2] = {0,0};
char playerOneScoreText[3];
char playerTwoScoreText[3];

char currentQuestion[20];
bool currentQuestionIsCorrect;
char roundResultText[20];

// Game/Application states
typedef enum {STATE_STARTING_UP = -1, STATE_IDLE = 0, STATE_PLAYING = 1, STATE_PLAYING_WAIT = 2, STATE_GAMEOVER = 3} GameState;
// Map states to their corresponding functions
void (*stateTable[])() = {menuLoop, gameLoop, gameWaitLoop, gameOverLoop};
void (*stateTableRender[])() = {menuRender, gameRender, gameWaitRender, gameOverRender};
GameState gameState = STATE_STARTING_UP;
bool gameOver = false;

void setup() {
  while (!Serial) { };
  Serial.begin(19200);

  playerOne.begin();
  playerTwo.begin();

  oled.begin();

  randomSeed(analogRead(2));

  enterState(STATE_IDLE);

  Serial.println("Setup done..");
}

void reset() {
  questionIndex = 0;
  playerScores[0] = 0;
  playerScores[1] = 0;
  
  setNextQuestion();
}

void setNextQuestion() {
  questionIndex++;

  int valueA = random(1,9);
  int valueB = random(1,9);
  
  char operation[1] = "*";
  int randomOp = random(3);
  int answer;
  switch (randomOp) {
    case 0:
      operation[0] = '-';
      answer = valueA - valueB;
      break;
    case 1:
      operation[0] = '+';
      answer = valueA + valueB;
      break;
    case 2:
      operation[0] = 'x';
      answer = valueA * valueB;
      break;
  }

  char aChar[5] = "";
  char bChar[5] = "";

  itoa(valueA, aChar, 10);
  itoa(valueB, bChar, 10);

  char str[20];
  strcpy(currentQuestion, "");
  strcat(currentQuestion, aChar);
  strcat(currentQuestion, operation);
  strcat(currentQuestion, bChar);  
  strcat(currentQuestion,"=");
  
  if (random(2) == 0) {
    char answerAsChar[] = "";
    itoa(answer, answerAsChar, 10);
    strcat(currentQuestion, answerAsChar);
    currentQuestionIsCorrect = true;  
  } else {
    char answerAsChar[] = "";
    int wrongAnswer = answer;
  
    while (wrongAnswer == answer) {
      wrongAnswer = answer + random(-2,2);
    }
    itoa(wrongAnswer, answerAsChar, 10);
    strcat(currentQuestion, answerAsChar);    
    currentQuestionIsCorrect = false;
  }

  lastQuestionUpdateMs = currentMs;
}

bool shouldTimeoutQuestion() {
  if ((currentMs - lastQuestionUpdateMs) > QUESTION_TIMEOUT_MS) {
    return true;
  }
  return false;
}

bool enterState(GameState nextState) {
  if (gameState == nextState) {
    return false;
  }

  if (gameState != STATE_PLAYING_WAIT && nextState == STATE_PLAYING) {
    reset();
  }

  gameState = nextState;  
  return true;
}

void loop() {
  currentMs = millis();
  playerOne.read();
  playerTwo.read();
  stateTable[gameState]();
  draw();
}

void draw() {
  oled.firstPage();
  do {
    stateTableRender[gameState]();
  } while ( oled.nextPage() );
}

void menuLoop() {
  if (playerOne.wasPressed() || playerTwo.wasPressed()){
     enterState(STATE_PLAYING);
  }
}

void menuRender() {
  if (currentMs % 2000 > 1000) { //blink text
    oled.setFont(u8g_font_unifont);
    oled.drawStr(0,24, "PRESS TO START");
  }
}

void gameLoop() {
  int player = 0;
  
  if (playerOne.wasPressed()) {
    player = 1;
  } else if (playerTwo.wasPressed()) {
    player = 2;
  }

  if (player > 0) {
    if (currentQuestionIsCorrect) {
      playerScores[player-1] += 1;
      strcpy(roundResultText, player==1?"POINT P1":"POINT P2");
    } else {
      playerScores[player-1] -= 2;
      strcpy(roundResultText, player==1?"MISS P1":"MISS P2");
    }
    enterState(STATE_PLAYING_WAIT);
  }

  if (shouldTimeoutQuestion()) {
    setNextQuestion();
  }
}

void gameRender() {
  oled.setFont(u8g_font_gdr20);
  oled.drawStr(25,24,currentQuestion);
  
  drawPlayerScores();
}

void drawPlayerScores() {
  oled.setFont(u8g_font_gdr20);
  oled.drawStr(10,    60, itoa(playerScores[0],playerOneScoreText, 10));
  oled.drawStr(128-26,60, itoa(playerScores[1],playerTwoScoreText, 10));
}

void gameWaitLoop() {
  if (playerScores[0] >= WIN_SCORE || playerScores[1] >= WIN_SCORE) {
    enterState(STATE_GAMEOVER);
  }

  if (shouldTimeoutQuestion()) {  
    setNextQuestion();
    enterState(STATE_PLAYING);
  }
}

void gameWaitRender() {
  oled.setFont(u8g_font_gdr20);
  oled.drawStr(0,24,roundResultText);

  drawPlayerScores();
}

void gameOverLoop() {
  if (playerOne.wasPressed() || playerTwo.wasPressed()){
     enterState(STATE_IDLE);
  }
}

void gameOverRender() {
  oled.setFont(u8g_font_gdr20);

  if (playerScores[0] > playerScores[1]) {
    oled.drawStr(0,24,"P1 WINS!");
  } else {
    oled.drawStr(0,24,"P2 WINS!");
  }
}