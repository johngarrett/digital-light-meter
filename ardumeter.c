/*
  ArduMeter (Arduino incident light meter) version 2
  by Alan Wang

  BH1750/BH1750FVI digital light intensity sensor:
  Library: https://github.com/claws/BH1750
  VCC -> 3.3V
  GND -> GND
  SDA -> pin A4
  SCL -> pin A5
  ADD -> (none)

  0.96" SSD1306 OLED 128x64 display:
  Library: https://github.com/greiman/SSD1306Ascii
  VCC -> 3.3V
  GND -> GND
  SDA -> pin A4
  SCL -> pin A5

  1x push button
  leg 1 -> pin D2
  leg 2 -> GND

  2x 10 kOhms potentiometer (knob)
  leg 1 -> 3.3V
  leg 2 (middle) -> pin A0/A1 (max analog signal about 730-740)
  leg 3 -> GND

   ------------------------------ */

// Set this to true if you wish to get "standard" shutter settings
#define STANRARD_SHUTTER_SPEED  false

// Calibration constant (C) for incident light meters.
// see https://en.wikipedia.org/wiki/Light_meter#Calibration_constants
#define INCIDENT_CALIBRATION    330

// Max analog readings from potentiometers. Change it only if you are
// using different input voltage or potentiometers that are not 10K.
#define KNOB_MAX_ANALOG_READING 740

// Define pins
#define BUTTON_PIN              2  // D2
#define KNOB_APERTURE_PIN       0  // A0
#define KNOB_ISO_PIN            1  // A1

// Pre-defined aperture and ISO settings. Modify them for your own preference.
// Elements in the array have to be sorted from min to max.
const double APERATURE_TABLE[]  = {1.0, 1.4, 1.8, 2.0, 2.8, 3.5, 4.0, 4.5, 5.6, 6.3, 8.0, 11.0, 12.7, 16.0, 22.0, 32.0};
const int ISO_TABLE[]           = {6, 12, 25, 50, 100, 160, 200, 400, 800, 1600, 3200};

// The "shutterspeed table" array is used when STANRARD_SHUTTER_SPEED is set to true.
// 1/2 (0.5) seconds is expressed as 2, and 2 seconds is expressed as -2.
double SHUTTERSPEED_TABLE[]     = {-60, -30, -15, -8, -4, -2, -1, 2, 4, 8, 15, 30, 60, 125, 250, 500, 1000, 2000, 4000};

// If you are using older cameras which have different shutter speed settings,
// you can switch to the folloing array:
// double SHUTTERSPEED_TABLE[]    = {-1, 2, 5, 10, 25, 50, 100, 250, 500, 1000};

/* ------------------------------ */

#include <math.h>
#include <Wire.h>
#include <BH1750.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"

BH1750 bh1750;
SSD1306AsciiAvrI2c oled;

bool started = false;
int EV, apertureIndex, apertureIndexOld, isoIndex, isoIndexOld;
long lux;

void setup() {

  // initialize serial port
  Serial.begin(9600);
  Wire.begin();

  // prepare the shutterspeed table array if needed
  if (STANRARD_SHUTTER_SPEED) {
    for (int i = 0; i < (sizeof(SHUTTERSPEED_TABLE) / sizeof(double)); i++) {
      if (SHUTTERSPEED_TABLE[i] < 0) {
        SHUTTERSPEED_TABLE[i] = 1 / abs(SHUTTERSPEED_TABLE[i]);
      }
    }
  }

  // setup button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // set the BH1750 in BH1750_ONE_TIME_HIGH_RES_MODE_2
  bh1750.begin(0x21);

  // "initialize" BH1750 by using it once
  bh1750.readLightLevel();

  // initialize OLED
  oled.begin(&Adafruit128x64, 0x3c);
  oled.setFont(System5x7);
  oled.set2X();

  Serial.println("ArduMeter (Arduino incident light meter for camera) READY:");
  Serial.println("- Press button to measure light.");
  Serial.println("- Turn knobs to change aperature/ISO.");

  oled.clear();
  oled.println("ArduMeter");
  oled.println("READY");
  oled.println("press");
  oled.println("button");
}

void loop() {

  //read status from button and potentiometers
  getKnobIndex();

  if (buttonPressed()) { //measure light
    if (!started) started = true;
    displayExposureSetting(true);
    delay(150);

  } else { //change aperture/ISO settings
    if ((apertureIndex != apertureIndexOld || isoIndex != isoIndexOld) && started) {
      displayExposureSetting(false);
      delay(150);
    }
  }

  //record potentiometer previous status
  apertureIndexOld = apertureIndex;
  isoIndexOld = isoIndex;

  delay(50);
}

// read if the button is pressed (pulled low)
bool buttonPressed() {
  return !digitalRead(BUTTON_PIN);
}

// map knob analog readings to array indexes
void getKnobIndex() {
  apertureIndex = round(map(analogRead(KNOB_APERTURE_PIN), 0, KNOB_MAX_ANALOG_READING, 0, sizeof(APERATURE_TABLE) / sizeof(float)));
  isoIndex = round(map(analogRead(KNOB_ISO_PIN), 0, KNOB_MAX_ANALOG_READING, 0, sizeof(ISO_TABLE) / sizeof(int)));
}

// measure/calculate and display exposure settings
void displayExposureSetting(bool measureNewEV) {

  double aperature = APERATURE_TABLE[apertureIndex];
  int iso = ISO_TABLE[isoIndex];

  // measure light level (illuminance) and get a new lux value
  if (measureNewEV) {
    lux = bh1750.readLightLevel();
    Serial.print("Measured illuminance = ");
    Serial.print(lux);
    Serial.println(" lux");
  }

  //calculate EV
  EV = log10(lux * iso / INCIDENT_CALIBRATION) / log10(2);

  if (isfinite(EV)) { //calculate shutter speed if EV is neither NaN nor infinity

    // calculate shutter speed
    double shutterspeed = (pow(2, EV) / pow(aperature, 2));

    // choose standard shutter speed if needed
    if (STANRARD_SHUTTER_SPEED) {
      for (int i = 0; i < (sizeof(SHUTTERSPEED_TABLE) / sizeof(double)) - 1; i++) {
        if (shutterspeed >= SHUTTERSPEED_TABLE[i] && shutterspeed <= SHUTTERSPEED_TABLE[i + 1]) {
          if (abs(shutterspeed - SHUTTERSPEED_TABLE[i]) <= abs(shutterspeed) - SHUTTERSPEED_TABLE[i + 1]) {
            shutterspeed = SHUTTERSPEED_TABLE[i];
          } else {
            shutterspeed = SHUTTERSPEED_TABLE[i + 1];
          }
          break;
        }
      }
    }

    // output result to serial port
    Serial.print("Exposure settings: EV = ");
    Serial.print(EV);
    Serial.print(", ISO = ");
    Serial.print(iso);
    Serial.print(", aperture = f/");
    Serial.print(aperature, 1);
    Serial.print(", ");
    Serial.print("shutter speed = ");
    if (shutterspeed > 1) {
      Serial.print("1/");
      Serial.print(shutterspeed);
    } else {
      Serial.print((1 / shutterspeed));
    }
    Serial.println("s");

    // output result to OLED
    oled.clear();
    oled.print("EV: ");
    oled.println(EV, 1);
    oled.print("ISO ");
    oled.println(iso);
    oled.print("-- f/");
    oled.println(aperature, 1);
    oled.print("-- ");
    if (shutterspeed > 1) {
      oled.print("1/");
      oled.print(shutterspeed, 0);
    } else {
      oled.print((1 / shutterspeed), 0);
    }
    oled.println("s");

  } else {
    Serial.println("Exposure out of bounds");
    oled.clear();
    oled.println("Exposure");
    oled.println("value");
    oled.println("out of");
    oled.println("bounds");
  }
}