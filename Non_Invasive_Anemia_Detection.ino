/*
  =====================================================================
  Non-Invasive Anemia Detection System Using IR Sensor
  =====================================================================
  Author  : Srinivasan G (Biomedical Engineering)
  Date    : Feb 2024

  Description:
  This sketch reads an IR sensor placed on the fingertip, converts the
  analog reading into an estimated hemoglobin (Hb) level using a linear
  calibration model, classifies the result as ANEMIC / NOT ANEMIC, and
  displays both values on a 16x2 character LCD.

  ---------------------------------------------------------------------
  HARDWARE CONNECTIONS (as per project wiring diagram)
  ---------------------------------------------------------------------
  IR SENSOR MODULE
    VCC  -> 5V
    GND  -> GND
    OUT  -> A0 (Arduino Analog In)

  16x2 LCD (4-bit mode)
    RS   -> Pin 12
    EN   -> Pin 11
    D4   -> Pin 5
    D5   -> Pin 4
    D6   -> Pin 3
    D7   -> Pin 2
    VSS  -> GND
    VDD  -> 5V
    (LCD R/W pin tied to GND, contrast via potentiometer to V0)

  ---------------------------------------------------------------------
  IMPORTANT CALIBRATION NOTE
  ---------------------------------------------------------------------
  The IR sensor gives a raw ADC value (0-1023) related to the amount of
  light transmitted through the fingertip. This raw value must be
  mapped to an actual Hb (g/dL) reading using reference blood-test
  data (invasive lab results) collected from multiple test subjects.

  The constants IR_RAW_MIN, IR_RAW_MAX, HB_MIN, HB_MAX below define a
  simple two-point linear calibration:

      Hb = HB_MIN + (rawValue - IR_RAW_MIN) * (HB_MAX - HB_MIN)
                    ------------------------------------------
                              (IR_RAW_MAX - IR_RAW_MIN)

  You MUST replace IR_RAW_MIN / IR_RAW_MAX with values obtained by
  placing the sensor on fingers with known lab Hb values and recording
  the corresponding raw analog readings. The values below are
  placeholders for demonstration only and will not give clinically
  accurate results until calibrated.
  =====================================================================
*/

#include <LiquidCrystal.h>

// ---------------- LCD Setup (RS, EN, D4, D5, D6, D7) ----------------
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// ---------------- Pin Definitions ------------------------------------
const int IR_SENSOR_PIN = A0;

// ---------------- Calibration Constants -------------------------------
// Replace these four values with your own two-point calibration data.
const int   IR_RAW_MIN = 300;    // raw ADC value at LOW Hb reference point
const int   IR_RAW_MAX = 700;    // raw ADC value at HIGH Hb reference point
const float HB_MIN     = 6.0;    // lab Hb (g/dL) corresponding to IR_RAW_MIN
const float HB_MAX     = 17.0;   // lab Hb (g/dL) corresponding to IR_RAW_MAX

// Anemia threshold (WHO general adult screening cut-off, g/dL)
// Adjust to 13.0 (male) / 12.0 (female) if gender-specific screening is needed.
const float ANEMIA_THRESHOLD = 12.0;

// ---------------- Sampling Settings -----------------------------------
const int NUM_SAMPLES = 20;      // number of readings averaged per measurement
const int SAMPLE_DELAY_MS = 10;  // delay between individual samples

void setup() {
  Serial.begin(9600);

  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Anemia Detector");
  lcd.setCursor(0, 1);
  lcd.print("  Initializing  ");
  delay(2000);
  lcd.clear();
}

void loop() {
  int rawValue = readAveragedIRValue();
  float hbLevel = convertToHb(rawValue);
  bool isAnemic = (hbLevel < ANEMIA_THRESHOLD);

  displayResult(hbLevel, isAnemic);
  printToSerial(rawValue, hbLevel, isAnemic);

  delay(3000); // update every 3 seconds
}

// -----------------------------------------------------------------------
// Reads the IR sensor multiple times and returns the averaged raw value.
// Averaging reduces noise from ambient light and finger movement.
// -----------------------------------------------------------------------
int readAveragedIRValue() {
  long total = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    total += analogRead(IR_SENSOR_PIN);
    delay(SAMPLE_DELAY_MS);
  }
  return (int)(total / NUM_SAMPLES);
}

// -----------------------------------------------------------------------
// Converts the raw IR ADC reading into an estimated Hb value (g/dL)
// using linear interpolation based on the calibration constants.
// -----------------------------------------------------------------------
float convertToHb(int rawValue) {
  float hb = HB_MIN + ((float)(rawValue - IR_RAW_MIN) * (HB_MAX - HB_MIN)) /
                       (float)(IR_RAW_MAX - IR_RAW_MIN);

  // Clamp result to a physiologically reasonable range
  if (hb < 3.0) hb = 3.0;
  if (hb > 20.0) hb = 20.0;

  return hb;
}

// -----------------------------------------------------------------------
// Displays the Hb level and status on the 16x2 LCD, matching the
// exact output format used in the project documentation:
//   Line 1: HB LEVEL: xx.x g/dL
//   Line 2: STATUS: ANEMIC / NOT ANEMIC
// -----------------------------------------------------------------------
void displayResult(float hbLevel, bool isAnemic) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("HB LEVEL: ");
  lcd.print(hbLevel, 1);
  lcd.print(" g/dL");

  lcd.setCursor(0, 1);
  if (isAnemic) {
    lcd.print("STATUS: ANEMIC");
  } else {
    lcd.print("STATUS: NOT ANEMIC");
  }
}

// -----------------------------------------------------------------------
// Sends the same result to the Serial Monitor for logging / Python-based
// analysis (as described in the project methodology).
// -----------------------------------------------------------------------
void printToSerial(int rawValue, float hbLevel, bool isAnemic) {
  Serial.print("Raw IR Value: ");
  Serial.print(rawValue);
  Serial.print(" | Estimated Hb: ");
  Serial.print(hbLevel, 1);
  Serial.print(" g/dL | Status: ");
  Serial.println(isAnemic ? "ANEMIC" : "NOT ANEMIC");
}
