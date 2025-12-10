/**********************************************************************
 * TASKBOARD + SCOREBOARD
 * - LCD shows tasks, mode, messages, timers
 * - 4-digit 7-seg shows SCORE
 *
 * MODES:
 *   0 = MANUAL    (deliver to house, timed)
 *   1 = AUTO MAZE (escape, time counted up)
 *   2 = STOP
 *
 * IR:
 *   1 = Manual mode (score reset)
 *   2 = Auto maze mode
 *   3 = Stop mode
 *   UP = Start manual delivery round
 *   POWER = Confirm delivery (manual) / Finish maze (auto)
 **********************************************************************/

#include <LiquidCrystal.h>
#include <IRremote.h>

/* ---------------- LCD & IR ---------------- */

LiquidCrystal lcd(7, 8, 9, 10, 11, 12);   

#define IR_PIN 1
IRrecv irrecv(IR_PIN);
decode_results results;

/* --- IR Codes --- */
#define IR_UP     0xFF02FD
#define IR_1      0xFF30CF   // Manual mode
#define IR_2      0xFF18E7   // Auto mode
#define IR_3      0xFF7A85   // Stop mode
#define IR_POWER  0xFFA25D   // Deliver / Finish
// (0xFFFFFFFF is repeat code, we ignore it)

/* ---------------- GAME STATE ---------------- */

// Modes: 0 = manual, 1 = auto maze, 2 = emergency stop
int mode = 0;

/* --- Manual Mode Variables --- */
const int deliveryTime = 30;      //seconds, for each manual delivery
unsigned long manualStartTime = 0;
bool manualTimerRunning = false;
bool manualRoundActive = false;

int currentHouse = 1;
int lastHouse = 0;

/* --- Auto Maze Mode Variables --- */
unsigned long mazeStartTime = 0;
bool mazeRunning = false;

/* --- Score --- */
int score = 0;   //0–99 (shown on 7-seg display)

/* ------------- 7-SEGMENT DEFINITIONS ------------- */

// segment pins (a,b,c,d,e,f,g)
const int SEG_A = 2;
const int SEG_B = 3;
const int SEG_C = 4;
const int SEG_D = 5;
const int SEG_E = 6;
const int SEG_F = 13;
const int SEG_G = A5;
const int DIG1 = A1;  
const int DIG2 = A2;
const int DIG3 = A3;
const int DIG4 = A4; 

//storing which number to show on each digit(0–9 or -1 for blank)
int scoreDigits[4] = {-1, -1, 0, 0};

unsigned long lastSegRefresh = 0;
byte currentDigitIndex = 0;

//segment patterns for digits 0–9
//This assumes: segments ON = HIGH, OFF = LOW, common cathode.
byte digitPatterns[10] = {
  // g f e d c b a  
  B00111111, // 0
  B00000110, // 1
  B01011011, // 2
  B01001111, // 3
  B01100110, // 4
  B01101101, // 5
  B01111101, // 6
  B00000111, // 7
  B01111111, // 8
  B01101111  // 9
};

/* ---------------------------------------------------------- */

void setup() {
  lcd.begin(16, 2);
  irrecv.enableIRIn();

  initSevenSeg();
  setScoreOn7Seg(score);    //show 00 at start

  welcomeScreen();
}

/* ---------------------------------------------------------- */

void loop() {

  // --- IR handling ---
  if (irrecv.decode(&results)) {
    uint32_t code = results.value;

    if (code != 0xFFFFFFFF) { //ignoring repeat
      handleIR(code);
    }

    irrecv.resume();
  }

  // --- Timers ---
  if (mode == 0 && manualTimerRunning) {
    updateManualCountdown();
  }

  if (mode == 1 && mazeRunning) {
    updateMazeTimer();
  }

  // --- Refresh 7-seg display frequently ---
  refreshSevenSeg();
}

/* ==================== IR LOGIC ==================== */

void handleIR(uint32_t code) {

  // ------- Mode Change --------
  if (code == IR_1) {
    // Manual mode
    mode = 0;
    manualTimerRunning = false;
    manualRoundActive = false;
    mazeRunning = false;

    // reset score in manual mode
    score = 0;
    setScoreOn7Seg(score);

    showManualScreen();
    return;
  }

  if (code == IR_2) {
    // Auto maze mode
    startAutoMode();
    return;
  }

  if (code == IR_3) {
    // Stop mode
    mode = 2;
    manualTimerRunning = false;
    mazeRunning = false;
    showStopScreen();
    return;
  }

  // ------- Mode-specific buttons --------

  //Start manual round: UP
  if (code == IR_UP && mode == 0 && !manualRoundActive) {
    startNewManualRound();
    return;
  }

  // POWER button: delivery / finish
  if (code == IR_POWER) {
    if (mode == 0 && manualTimerRunning) {
      handleManualDeliverySuccess();
    }
    else if (mode == 1 && mazeRunning) {
      handleMazeFinished();
    }
  }
}

/* ==================== WELCOME & MODE SCREENS ==================== */

void welcomeScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WELCOME TO :)");
  lcd.setCursor(0, 1);
  lcd.print("SANTA'S DELIVERY");
  delay(1500);
  showManualScreen();
}

void showManualScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MANUAL MODE");
  lcd.setCursor(0, 1);
  lcd.print("Press UP to Start");
}

void showStopScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("STOP MODE");
  lcd.setCursor(0, 1);
  lcd.print("Car Halted");
}

/* ==================== MANUAL MODE ==================== */

void startNewManualRound() {
  manualRoundActive = true;

  pickNewHouse();
  manualStartTime = millis();
  manualTimerRunning = true;
}

void pickNewHouse() {
  int house = random(1, 4);
  while (house == lastHouse) {
    house = random(1, 4);
  }

  lastHouse = house;
  currentHouse = house;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Deliver to:");
  lcd.setCursor(0, 1);
  lcd.print("HOUSE {");
  lcd.print(currentHouse);
  lcd.print("}");
}

void updateManualCountdown() {
  unsigned long elapsed = (millis() - manualStartTime) / 1000;
  int remaining = deliveryTime - elapsed;

  //show |Xs|
  lcd.setCursor(12, 1);
  lcd.print("|");
  lcd.print(max(remaining, 0));
  lcd.print("s|");

  if (remaining < 0) {
    manualTimerRunning = false;
    manualRoundActive = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("TIME UP!");
    lcd.setCursor(0, 1);
    lcd.print("Delivery Failed");
    delay(2000);
    showManualScreen();
  }
}

void handleManualDeliverySuccess() {
  //stop timer
  manualTimerRunning = false;
  manualRoundActive = false;

  //increase score (clamp 0–99)
  score++;
  if (score > 99) score = 99;
  setScoreOn7Seg(score);

  //show success message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DELIVERED!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
  delay(2000);

  //automatically start next round
  startNewManualRound();
}

/* ==================== AUTO MAZE MODE ==================== */

void startAutoMode() {
  mode = 1;
  manualTimerRunning = false;
  manualRoundActive = false;

  mazeStartTime = millis();
  mazeRunning = true;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AUTOMATIC MODE");
  lcd.setCursor(0, 1);
  lcd.print("ESCAPE MAZE |0s|");
}

void updateMazeTimer() {
  unsigned long elapsed = (millis() - mazeStartTime) / 1000;

  lcd.setCursor(12, 1);
  lcd.print("|");
  lcd.print(elapsed);
  lcd.print("s| ");
}

void handleMazeFinished() {
  mazeRunning = false;
  unsigned long elapsed = (millis() - mazeStartTime) / 1000;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MAZE COMPLETED");
  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  lcd.print(elapsed);
  lcd.print("s");

  //to run again, press '2' (Auto mode) manually
}

/* ==================== 7-SEGMENT SCORE DISPLAY ==================== */

void initSevenSeg() {
  pinMode(SEG_A, OUTPUT);
  pinMode(SEG_B, OUTPUT);
  pinMode(SEG_C, OUTPUT);
  pinMode(SEG_D, OUTPUT);
  pinMode(SEG_E, OUTPUT);
  pinMode(SEG_F, OUTPUT);
  pinMode(SEG_G, OUTPUT);

  pinMode(DIG1, OUTPUT);
  pinMode(DIG2, OUTPUT);
  pinMode(DIG3, OUTPUT);
  pinMode(DIG4, OUTPUT);

  // start all off
  digitalWrite(DIG1, LOW);
  digitalWrite(DIG2, LOW);
  digitalWrite(DIG3, LOW);
  digitalWrite(DIG4, LOW);
}

void setSegments(byte pattern) {
  //bit 0 -> a, 1 -> b, ..., 6 -> g
  digitalWrite(SEG_A, pattern & B00000001 ? HIGH : LOW);
  digitalWrite(SEG_B, pattern & B00000010 ? HIGH : LOW);
  digitalWrite(SEG_C, pattern & B00000100 ? HIGH : LOW);
  digitalWrite(SEG_D, pattern & B00001000 ? HIGH : LOW);
  digitalWrite(SEG_E, pattern & B00010000 ? HIGH : LOW);
  digitalWrite(SEG_F, pattern & B00100000 ? HIGH : LOW);
  digitalWrite(SEG_G, pattern & B01000000 ? HIGH : LOW);
}

//score: 0–99, show on right two digits, left two blank
void setScoreOn7Seg(int scoreValue) {
  if (scoreValue < 0) scoreValue = 0;
  if (scoreValue > 99) scoreValue = 99;

  int tens = scoreValue / 10;
  int ones = scoreValue % 10;

  //left two digits blank (-1)
  scoreDigits[0] = -1;
  scoreDigits[1] = -1;
  scoreDigits[2] = tens;
  scoreDigits[3] = ones;
}

void refreshSevenSeg() {
  unsigned long now = millis();
  if (now - lastSegRefresh < 3) return;  //about ~300Hz / 4 digits
  lastSegRefresh = now;

  //turn all digits OFF first
  digitalWrite(DIG1, LOW);
  digitalWrite(DIG2, LOW);
  digitalWrite(DIG3, LOW);
  digitalWrite(DIG4, LOW);

  int digitVal = scoreDigits[currentDigitIndex];

  if (digitVal >= 0 && digitVal <= 9) {
    setSegments(digitPatterns[digitVal]);
  } else {
    //blank
    setSegments(0);
  }

  // enable the current digit (assuming common cathode, active HIGH on digit pins)
  switch (currentDigitIndex) {
    case 0: digitalWrite(DIG1, HIGH); break;
    case 1: digitalWrite(DIG2, HIGH); break;
    case 2: digitalWrite(DIG3, HIGH); break;
    case 3: digitalWrite(DIG4, HIGH); break;
  }

  currentDigitIndex = (currentDigitIndex + 1) % 4;
}

