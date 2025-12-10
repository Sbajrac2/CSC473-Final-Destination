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
#include "LEDStripController.h"


/* ---- LED Strip ---- */
#define LED_I2C_ADDR 0x20
#define LED_COUNT 10
LEDStripController strip(LED_I2C_ADDR, LED_COUNT, TYPE_GRB);


/* ---- FUNCTION DECLARATION ---- */
void setModeColor(uint8_t r, uint8_t g, uint8_t b);


/* ---- IR Remote ---- */
#define IR_PIN 9
IRrecv irrecv(IR_PIN);
decode_results results;


#define IR_UP       0xFF02FD
#define IR_DOWN     0xFF9867
#define IR_LEFT     0xFFE01F
#define IR_RIGHT    0xFF906F
#define IR_OK       0xFFA857
#define IR_1        0xFF30CF
#define IR_2        0xFF18E7
#define IR_3        0xFF7A85


unsigned long lastIRTime = 0;
#define IR_TIMEOUT 150


/* ---- Car Pins ---- */
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


/* ---- Thresholds ---- */
#define BLACK_THRESHOLD 500
#define ULTRASONIC_STOP_DISTANCE 15.0


/* ================================================================
                       LED COLOR FUNCTION
  ================================================================ */
void setModeColor(uint8_t r, uint8_t g, uint8_t b) {
 strip.setAllLedsColor(r, g, b);
 strip.show();
}


/* ================================================================
                         MOTOR FUNCTION
  ================================================================ */
void motorRun(int L, int R) {
 int dirL = (L > 0 ? LOW : HIGH);
 int dirR = (R > 0 ? HIGH : LOW);


 L = constrain(abs(L), 0, 255);
 R = constrain(abs(R), 0, 255);


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


void readSensors(int &L, int &C, int &R) {
 L = analogRead(PIN_TRACKING_LEFT);
 C = analogRead(PIN_TRACKING_CENTER);
 R = analogRead(PIN_TRACKING_RIGHT);
}


/* ================================================================
                    AUTO MODE 
  ================================================================ */
void autoMaze() {
 int L, C, R;
 float dist = readDistance();
 readSensors(L, C, R);


 bool LB = (L > BLACK_THRESHOLD);
 bool CB = (C > BLACK_THRESHOLD);
 bool RB = (R > BLACK_THRESHOLD);


 /* STOP on obstacle */
 if (dist > 0 && dist < ULTRASONIC_STOP_DISTANCE) {
   motorRun(0, 0);
   return;
 }


 /* RULE: ALL BLACK → straight */
 if (LB && CB && RB) {
   motorRun(150, 150);
   return;
 }


 /* RULE: LEFT WHITE → turn RIGHT */
 if (!LB) {
   motorRun(150, -150);
   return;
 }


 /* RULE: RIGHT WHITE → turn LEFT */
 if (!RB) {
   motorRun(-150, 150);
   return;
 }


 /* RULE: CENTER WHITE but sides black → slight straight */
 if (!CB && LB && RB) {
   motorRun(130, 130);
   return;
 }


 /* Default fallback → straight */
 motorRun(150, 150);
}


/* ================================================================
                            SETUP
  ================================================================ */
int carMode = 0;


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


 strip.begin();
 strip.show();


 irrecv.enableIRIn();


 setModeColor(0, 255, 0); // Manual mode green
}


/* ================================================================
                            MAIN LOOP
  ================================================================ */
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


 /* AUTO MODE */
 if (carMode == 1) autoMaze();


 /* STOP MODE */
 if (carMode == 2) {
   motorRun(0,0);
   setBuzzer(true);
 }


 /* MANUAL ultrasonic warning */
 if (carMode == 0) {
   float d = readDistance();
   setBuzzer(d > 0 && d < ULTRASONIC_STOP_DISTANCE);
 }


 /* IR timeout safety */
 if (carMode == 0 && (millis() - lastIRTime > IR_TIMEOUT)) {
   motorRun(0,0);
 }
}





