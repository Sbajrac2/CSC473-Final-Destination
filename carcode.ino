/**********************************************************************
 * CAR WITH:
 * - Manual Mode
 * - Automatic Maze Mode (line sensors + ultrasonic)
 * - Emergency Stop Mode
 * - LED strip color per mode (WS2812B via LEDStripController)
 *
 * LEDs:
 * Manual = GREEN
 * Automatic = BLUE
 * Stop = RED
 **********************************************************************/

#include <IRremote.h>
#include "LEDStripController.h"   // <--- your renamed Freenove LED library

/* ---- LED STRIP SETUP ---- */
#define LED_I2C_ADDR 0x20
#define LED_COUNT 10
LEDStripController strip(LED_I2C_ADDR, LED_COUNT, TYPE_GRB);

/* ---- IR REMOTE ---- */
#define IR_PIN 9
IRrecv irrecv(IR_PIN);
decode_results results;

#define IR_UP       0xFF02FD
#define IR_DOWN     0xFF9867
#define IR_LEFT     0xFFE01F
#define IR_RIGHT    0xFF906F
#define IR_OK       0xFFA857
#define IR_1        0xFF30CF   // Manual mode
#define IR_2        0xFF18E7   // Automatic mode
#define IR_3        0xFF7A85   // Stop mode

/* ---- IR SAFETY TIMEOUT ---- */
unsigned long lastIRTime = 0;
#define IR_TIMEOUT 150   // ms → stops car when no IR signal

/* ---- CAR PINS ---- */
#define PIN_DIRECTION_LEFT  4
#define PIN_DIRECTION_RIGHT 3
#define PIN_MOTOR_PWM_LEFT  6
#define PIN_MOTOR_PWM_RIGHT 5

#define PIN_SONIC_TRIG 7
#define PIN_SONIC_ECHO 8

#define PIN_TRACKING_LEFT   A1
#define PIN_TRACKING_CENTER A2
#define PIN_TRACKING_RIGHT  A3

#define PIN_BUZZER A0

#define BLACK_THRESHOLD 250
#define ULTRASONIC_STOP_DISTANCE 15.0

/* ---- MOTOR ---- */
#define MOTOR_PWM_DEAD 10
int currentL = 0, currentR = 0;

/* ---- MODE ---- */
int carMode = 0;   // 0 = manual, 1 = auto, 2 = stop

/* ---- HELPERS ---- */
void motorRun(int L, int R) {
  int dirL = (L > 0 ? LOW : HIGH);
  int dirR = (R > 0 ? HIGH : LOW);

  L = constrain(abs(L), 0, 255);
  R = constrain(abs(R), 0, 255);

  if (L < MOTOR_PWM_DEAD) L = 0;
  if (R < MOTOR_PWM_DEAD) R = 0;

  currentL = L;
  currentR = R;

  digitalWrite(PIN_DIRECTION_LEFT, dirL);
  digitalWrite(PIN_DIRECTION_RIGHT, dirR);
  analogWrite(PIN_MOTOR_PWM_LEFT, L);
  analogWrite(PIN_MOTOR_PWM_RIGHT, R);
}

void setBuzzer(bool on) {
  digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
}

float readDistance() {
  digitalWrite(PIN_SONIC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_SONIC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_SONIC_TRIG, LOW);

  long duration = pulseIn(PIN_SONIC_ECHO, HIGH, 25000);
  if (duration == 0) return -1;

  return duration * 0.0343 / 2.0;
}

void setModeColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setAllLedsColor(r, g, b);
  strip.show();
}

void readSensors(int &L, int &C, int &R) {
  L = analogRead(PIN_TRACKING_LEFT);
  C = analogRead(PIN_TRACKING_CENTER);
  R = analogRead(PIN_TRACKING_RIGHT);
}

/* ---- AUTOMATIC MAZE MODE ---- */
void autoMaze() {
  int L, C, R;
  float dist = readDistance();
  readSensors(L, C, R);

  if (dist > 0 && dist < ULTRASONIC_STOP_DISTANCE) {
    motorRun(0, 0);
    return;
  }

  if (L > BLACK_THRESHOLD && C > BLACK_THRESHOLD && R > BLACK_THRESHOLD) {
    motorRun(-150, -150);
    delay(200);
    motorRun(150, -150);
    delay(300);
    return;
  }

  if (L < BLACK_THRESHOLD && C < BLACK_THRESHOLD && R < BLACK_THRESHOLD) {
    motorRun(150, -150);
    delay(250);
    return;
  }

  if (C < BLACK_THRESHOLD) {
    motorRun(180, 180);
  } else if (L < BLACK_THRESHOLD) {
    motorRun(-120, 120);
  } else if (R < BLACK_THRESHOLD) {
    motorRun(120, -120);
  } else {
    motorRun(180, 180);
  }
}

/* ---- SETUP ---- */
void setup() {
  pinMode(PIN_DIRECTION_LEFT, OUTPUT);
  pinMode(PIN_DIRECTION_RIGHT, OUTPUT);
  pinMode(PIN_MOTOR_PWM_LEFT, OUTPUT);
  pinMode(PIN_MOTOR_PWM_RIGHT, OUTPUT);

  pinMode(PIN_SONIC_TRIG, OUTPUT);
  pinMode(PIN_SONIC_ECHO, INPUT);

  pinMode(PIN_TRACKING_LEFT, INPUT);
  pinMode(PIN_TRACKING_CENTER, INPUT);
  pinMode(PIN_TRACKING_RIGHT, INPUT);

  pinMode(PIN_BUZZER, OUTPUT);
  setBuzzer(false);

  strip.begin();    // your renamed library
  strip.show();

  irrecv.enableIRIn();

  setModeColor(0, 255, 0);   // manual mode default
}

/* ---- MAIN LOOP ---- */
void loop() {

  if (irrecv.decode(&results)) {
    uint32_t code = results.value;
    lastIRTime = millis();

    switch(code) {

      case IR_1:
        carMode = 0;
        setModeColor(0,255,0);
        motorRun(0,0);
        break;

      case IR_2:
        carMode = 1;
        setModeColor(0,0,255);
        motorRun(0,0);
        break;

      case IR_3:
        carMode = 2;
        setModeColor(255,0,0);
        motorRun(0,0);
        break;

      case IR_OK:
        setBuzzer(true);
        break;

      case IR_UP:
        if (carMode == 0) motorRun(180,180);
        break;

      case IR_DOWN:
        if (carMode == 0) motorRun(-180,-180);
        break;

      case IR_LEFT:
        if (carMode == 0) motorRun(-150,150);
        break;

      case IR_RIGHT:
        if (carMode == 0) motorRun(150,-150);
        break;
    }

    irrecv.resume();
  }

  /* Manual Mode Ultrasonic Warning */
  if (carMode == 0) {
    float d = readDistance();
    setBuzzer(d > 0 && d < ULTRASONIC_STOP_DISTANCE);
  }

  /* Automatic Maze */
  if (carMode == 1) {
    autoMaze();
  }

  /* Emergency Stop */
  if (carMode == 2) {
    motorRun(0,0);
    setBuzzer(true);
  }

  /* IR Timeout Safety */
  if (carMode == 0 && (millis() - lastIRTime > IR_TIMEOUT)) {
    motorRun(0,0);
  }
}
