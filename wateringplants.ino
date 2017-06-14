/*
 *            Automatic Plant Watering
 * Waters 4 plants evenly 2 times every week. Moisture sensors 
 * indicate if extra water is needed. Potentiometer enables manual
 * watering. With DEBUG defined sensor values and time can be monitored.
 * 
 * - 1 x Arduino Uno
 * - 1 x DC 5v pump
 * - 1 x Driver module (IRF-transistor with diode)
 * - 1 x Potentiometer
 * - 4 x Moisture sensors
 * 
 * Code in progress, Truls Nyberg.
*/

#include <Time.h>
#include <TimeAlarms.h>

//#define DEBUG
#include "myDebug.h"

const int POT            = A0;
const int MOISTURE_1     = A1;
const int MOISTURE_2     = A2;
const int MOISTURE_3     = A3;
const int MOISTURE_4     = A4;
const int PUMP           = 9;

const int defaultDelay    = 6000;   // Delay when no extra water is needed
const int moistLimit      = 400;    // Sensor value when extra water is needed
const int delayLimit      = 10000;  // Maximum extra delay when extra water is needed

int moistVal1   = 0;
int moistVal2   = 0;
int moistVal3   = 0;
int moistVal4   = 0;
int avgMoistVal = 0;

int moistureDelay = 0;
int pumpSpeed     = 0;
int potVal        = 0;

volatile int measureTimer  = 0;
volatile int waterTimer    = 0;
volatile int debugTimer    = 0;

void setup() {
  pinMode(PUMP, OUTPUT);
  pinMode(POT, INPUT);
  pinMode(MOISTURE_1, INPUT);
  pinMode(MOISTURE_2, INPUT);
  pinMode(MOISTURE_3, INPUT);
  pinMode(MOISTURE_4, INPUT);

  setTime(19, 37, 0, 5, 6, 17);   // Set time to 19:37:00, 5th of June 2017

#ifdef DEBUG
  Serial.begin(9600);
  Alarm.timerRepeat(1, setDebugTimer);

  DEBUG_PRINTLN("Time set to Monday 19:37:00, 5th of June 2017.");
  DEBUG_PRINTLN("Measuring moisture level 17:59 every Tuesday and 12:59 every Saturday.");
  DEBUG_PRINTLN("Watering plants 18:00 every Tuesday and every Saturday 13:00.");
#endif

  Alarm.alarmRepeat(dowTuesday, 17, 59, 00, setMeasureTimer);
  Alarm.alarmRepeat(dowTuesday, 18, 00, 00, setWaterTimer);
  Alarm.alarmRepeat(dowSaturday, 12, 59, 00, setMeasureTimer);
  Alarm.alarmRepeat(dowSaturday, 13, 00, 00, setWaterTimer);
}

void loop() {

  // Manually start the pump with potentiometer
  potVal = analogRead(POT);           // Reads value 0-1023
  pumpSpeed = potVal / 4;             // PWM value 0-255
  if (pumpSpeed > 200) {
    pumpSpeed = 200;                  // Limit the speed
  }
  analogWrite(PUMP, pumpSpeed);

  if (pumpSpeed > 0) {
    DEBUG_PRINT(pumpSpeed);
    DEBUG_PRINT(" ");
  }

  Alarm.delay(0);   // Enable timer interrupts
  
  // Measure moisture in plants if timer has interuppted
  if (measureTimer == 1) {
    measureMoisture();
    measureTimer = 0;
  }

  // Water plants if timer has interuppted
  if (waterTimer == 1) {
    waterPlants();
    waterTimer = 0;
  }

  // Output time in debug mode
  if (debugTimer == 1) {
    DEBUG_PRINT("Time:");
    DEBUG_PRINT(hour());
    printDigits(minute());
    printDigits(second());
    DEBUG_PRINTLN();

    debugTimer = 0;
  }
}

/////////// INTERRUPTS
void setMeasureTimer() {
  measureTimer = 1;
}

void setWaterTimer() {
  waterTimer = 1;
}

void setDebugTimer() {
  debugTimer = 1;
}
////////////////////////////

/////////// FUNCTIONS
void measureMoisture() {
  DEBUG_PRINTLN("Measuring moisture levels...");

  moistVal1 = analogRead(MOISTURE_1);
  moistVal2 = analogRead(MOISTURE_2);
  moistVal3 = analogRead(MOISTURE_3);
  moistVal4 = analogRead(MOISTURE_4);

  // TODO: Add a check if sensors are defect ( value = 0? ) to
  // aviod lowering the average when sensors are defect
  
  avgMoistVal = (moistVal1 + moistVal2 + moistVal3 + moistVal4) / 4;

  DEBUG_PRINTLN("Moisture levels (0-1023) plant 1-4:");
  DEBUG_PRINTLN(moistVal1);
  DEBUG_PRINTLN(moistVal2);
  DEBUG_PRINTLN(moistVal3);
  DEBUG_PRINTLN(moistVal4);
  DEBUG_PRINTLN("Average moisture level:");
  DEBUG_PRINTLN(avgMoistVal);

  if (avgMoistVal < moistLimit) {
    // Delay 0 to delayLimit ms extra if plants are dry
    moistureDelay = 25 * (moistLimit - avgMoistVal);
    //moistureDelay = (delayLimit/moistLimit) * (moistLimit - avgMoistVal); // Not tested
  }
  else {
    moistureDelay = 0;
  }

  DEBUG_PRINTLN("Moisture delay set to:");
  DEBUG_PRINTLN(moistureDelay);
}

void waterPlants() {
  DEBUG_PRINTLN("Watering plants...");
  DEBUG_PRINTLN("Watering time:");
  DEBUG_PRINTLN(defaultDelay + moistureDelay);
  
  pumpSpeed = 200;
  analogWrite(PUMP, pumpSpeed);
  
  delay(defaultDelay);
  delay(moistureDelay);
  
  pumpSpeed = 0;
  analogWrite(PUMP, pumpSpeed);
}

// For time output in debug mode
void printDigits(int digits)
{
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

////////////////////////////
