/*
  Heartbeat Sensor
  ----------------
  Created for PHABLABS 4.0

  Dependent library:
  Install the LiquidCrystal library in Documents/Arduino/libraries/

  Hackers:
  A row of holes on the PCB is intentionally left empty.
  The corresponding pins on the Arduino can be configured to do whatever you want...

  Please read this code, change it, try it and make it your own!
  
  created 03-07-2018
  modified 02-08-2018
  by Aidan Wyber, Science Centre Delft / TU Delft

  for questions on the code send me an email: aidanwyber@gmail.com
*/


// to include the library code
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 10, 9, 8, A2);

#define NLEDS 5
const int leds[] = {2, 3, 4, 5, 7};

#define BUZZER_PIN 6
#define PULSE_PIN A0
//#define BLINK_PIN 13
//#define FADE_PIN 5

#define THRESHOLD_FRAC 0.75

float currBPM;

#define NHIST 100
int pulseValHist[NHIST];

#define NBPMHIST 5
float BPMHist[NBPMHIST];

unsigned long prevBeatTime, currBeatTime; // in milliseconds
unsigned long lastLCDUpdate;

#define N_PREV_COMPARE 4

int minPulseVal, maxPulseVal;

boolean doMeasure = true;

void setup() {
  // set up the LCD's number of columns and rows:
  //  lcd.begin(32, 1);
  lcd.begin(16, 1);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("PHABLABS 4.0");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Heartbeat Sensor");
  delay(1000 - 20 * 4 * 2);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  //  pinMode(
  for (int i = 0; i < NLEDS; i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], LOW);
  }

  Serial.begin(115200);

  prevBeatTime = millis();
  currBeatTime = millis();

  int sd = 20;
  for (int j = 0; j < 4; j++) {
    for (int i = 0; i < NLEDS; i++) {
      digitalWrite(leds[i], HIGH);
      delay(sd);
      digitalWrite(leds[i], LOW);
    }
    for (int i = NLEDS - 1; i >= 0; i--) {
      digitalWrite(leds[i], HIGH);
      delay(sd);
      digitalWrite(leds[i], LOW);
    }
  }

  lcd.clear();

  for (int i = 0; i < NLEDS; i++) {
    digitalWrite(leds[i], HIGH);
  }
  delay(500);
  for (int i = 0; i < NLEDS; i++) {
    digitalWrite(leds[i], LOW);
  }
  delay(500);
}

void loop() {
  int pulseVal = analogRead(PULSE_PIN);

  updatePulseValHistory(pulseVal);

  computeMinMax();

  float diff = (float) maxPulseVal - minPulseVal;
  float threshold = minPulseVal + diff * THRESHOLD_FRAC;



  // actuate LEDs
  for (int i = 0; i < NLEDS; i++) {
    digitalWrite(leds[i], LOW);
  }

  float pulseValFrac = (pulseVal - minPulseVal) / diff;
  for (int i = 0; i < NLEDS + 1; i++) {
    int b = (float(i) / (NLEDS + 1) < pulseValFrac) ? 1 : 0;
    if (i > 0) {
      digitalWrite(leds[i - 1], b);
    }
  }




  // counter idle reading
  if (diff > 25) {

    // check if signal is rising and value is larger than threshold
    if (doMeasure && pulseVal > threshold) {
      doMeasure = false;

      if (pulseVal > pulseValHist[NHIST - 1 - N_PREV_COMPARE]) {
        // rising signal
        beat();
      } else {
        // falling signal
      }
    } else {
      doMeasure = true;
    }
  }

  Serial.print(pulseVal);
  Serial.print(", ");
  Serial.println(threshold);

  float avBPM = getBPMAverage();

  //lcd.clear();
  if (millis() - lastLCDUpdate > 200) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BPM: ");
    lcd.print(avBPM);
    lastLCDUpdate = millis();
  }

  delay(20);

}

void updatePulseValHistory(int pv) {
  for (int i = 0; i < NHIST - 1; i++) {
    pulseValHist[i] = pulseValHist[i + 1];
  }
  pulseValHist[NHIST - 1] = pv;
}

void computeMinMax() {
  minPulseVal = 1023; // start high
  maxPulseVal = 0; // start low
  for (int i = 0; i < NHIST; i++) {
    minPulseVal = min(minPulseVal, pulseValHist[i]);
    maxPulseVal = max(maxPulseVal, pulseValHist[i]);
  }

  //  Serial.print("min: ");
  //  Serial.print(minPulseVal);
  //  Serial.print(", max: ");
  //  Serial.println(maxPulseVal);
}

void updateBPMHistory(int b) {
  for (int i = 0; i < NBPMHIST - 1; i++) {
    BPMHist[i] = BPMHist[i + 1];
  }
  BPMHist[NBPMHIST - 1] = b;
}

float getBPMAverage() {
  float av = 0;
  for (int i = 0; i < NBPMHIST; i++) {
    av += BPMHist[i];
  }
  return av / NBPMHIST;
}


float timeToBPM(float t) {
  return 60.0 / t;
}

float bpmToTime(float bpm) {
  return 60.0 / bpm;
}


void beat() {
  prevBeatTime = currBeatTime;
  currBeatTime = millis();
  //Serial.println(1023);


  float timeDiff = (currBeatTime - prevBeatTime) * 0.001; // in seconds
  float currBPM = timeToBPM(timeDiff);
  if (currBPM < 160.0 && currBPM > 20.0) {
    updateBPMHistory(currBPM);
    tone(BUZZER_PIN, 10000, 2);
  }
}

