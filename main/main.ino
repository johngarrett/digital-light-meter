#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <i2cdetect.h>
#include <math.h> 
#include "src/BH1750/src/BH1750.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C /// 0x3D for 128x64, 0x3C for 128x32
#define SEL_PIN  5
#define POT_PIN   A5

// Calibration constant (C) for incident light meters.
// see https://en.wikipedia.org/wiki/Light_meter#Calibration_constants
// TODO: allow the user to set this
#define INCIDENT_CALIBRATION    330


const double APT_TABLE[]  = {1.0, 1.4, 1.8, 2.0, 2.8, 3.5, 4.0, 4.5, 5.6, 6.3, 8.0, 11.0, 12.7, 16.0, 22.0, 32.0};
const int ISO_TABLE[]     = {6, 12, 25, 50, 100, 160, 200, 400, 800, 1600, 3200, 6400};
const double SS_TABLE[]   = {-1, 2, 5, 10, 25, 50, 100, 250, 500, 1000};
const int C_TABLE[]       = { 250, 330 };
const int FL_TABLE[]      = { 28, 50 }; // focal length

const int apt_tbl_sz = sizeof(APT_TABLE) / sizeof(APT_TABLE[0]);
const int iso_tbl_sz = sizeof(ISO_TABLE) / sizeof(ISO_TABLE[0]);
const int ss_tbl_sz = sizeof(SS_TABLE) / sizeof(SS_TABLE[0]);
const int c_tbl_sz = sizeof(C_TABLE) / sizeof(C_TABLE[0]);
const int fl_tbl_sz = sizeof(FL_TABLE) / sizeof(FL_TABLE[0]);

enum mode { MODE_SS, MODE_SS_EDIT, MODE_APT, MODE_APT_EDIT, MODE_SETTINGS, MODE_SETTINGS_EDIT };
enum priority { APT_PRIO, SS_PRIO };

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

BH1750 lightMeter;

float lux;
double ev_delta = 0;
int apt_indx, iso_indx, ss_indx, c_indx, fl_indx = 0;
int sel_state = 0;
int pot_val = 0;

mode selected_mode = MODE_SETTINGS;
priority selected_prio = APT_PRIO;

void setup() {
  Serial.begin(9600);
//  while (!Serial) {
//
//  }
  Wire.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire)) {
    Serial.println(F("Began"));
  } else {
    Serial.println(F("Error initing lightmeter"));
  }
  Serial.println(F("BH1750 Test"));
  i2cdetect();

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  pinMode(SEL_PIN, INPUT);

  // TODO: read default values from sd card!
  double default_apt = 2.8;
  int default_iso = 200;
  double default_ss = 100;
  int default_c = 330;
  int default_fl = 50;


  // set indicies for each value
  for (int i = 0; i < apt_tbl_sz; ++i) {
    if (APT_TABLE[i] == default_apt) {
      apt_indx = i;
      break;
    }
  }

  for (int i = 0; i < iso_tbl_sz; ++i) {
    if (ISO_TABLE[i] == default_iso) {
      iso_indx = i;
      break;
    }
  }

  for (int i = 0; i < ss_tbl_sz; ++i) {
    if (SS_TABLE[i] == default_ss) {
      ss_indx = i;
      break;
    }
  }

  for (int i = 0; i < c_tbl_sz; ++i) {
    if (C_TABLE[i] == default_c) {
      c_indx = i;
      break;
    }
  }

  for (int i = 0; i < fl_tbl_sz; ++i) {
    if (FL_TABLE[i] == default_fl) {
      fl_indx = i;
      break;
    }
  }
}

void loop() {
  read_inputs();
  handle_inputs();
  read_lux();
  calculate_stats();
  display_info();
}

void read_inputs() {
  sel_state = digitalRead(SEL_PIN);
  pot_val = analogRead(POT_PIN);
}

void handle_inputs() {
  // pass control over to the settings file
  if (selected_mode == MODE_SETTINGS_EDIT) {
    handle_settings_input();
    return;
  }

  // switching between parent and child view
  if (sel_state == 1) {
    switch (selected_mode) {
      case MODE_APT:
        selected_mode = MODE_APT_EDIT;
        break;
      case MODE_APT_EDIT:
        selected_mode = MODE_APT;
        break;
      case MODE_SS:
        selected_mode = MODE_SS_EDIT;
        break;
      case MODE_SS_EDIT:
        selected_mode = MODE_SS;
        break;
      case MODE_SETTINGS:
        selected_mode = MODE_SETTINGS_EDIT;
        break;
    }
    delay(175); // to prevent jittering
  }

  // the user wants to set a value to be automatically decided
  if (selected_mode == MODE_SETTINGS || selected_mode == MODE_SS || selected_mode == MODE_APT) {
    // map pot val to { ISO, SS, APT } (3x for better error handling)
    switch (map(pot_val, 0, 1023, 1, 9)) {
      case 1:
      case 2:
      case 3:
        selected_mode = MODE_SETTINGS;
        break;
      case 4:
      case 5:
      case 6:
        selected_mode = MODE_SS;
        break;
      case 7:
      case 8:
      case 9:
        selected_mode = MODE_APT;
        break;
    }
  }

  if (selected_mode == MODE_SS_EDIT) {
    selected_prio = SS_PRIO;
    ss_indx = map(pot_val, 0, 1023, 0, ss_tbl_sz);
    if (ss_indx  == ss_tbl_sz) {
      ss_indx--;
    }
  }

  if (selected_mode == MODE_APT_EDIT) {
    selected_prio = APT_PRIO;
    apt_indx = map(pot_val, 0, 1023, 0, apt_tbl_sz);
    if (apt_indx == apt_tbl_sz) {
      apt_indx--;
    }
  }
}

void read_lux() {
  lux = lightMeter.readLightLevel();
}

// calculate necessary SS, APT based on prio and lux
void calculate_stats() {
  if (selected_prio == APT_PRIO) {
    // calculate SS 
    double apt = APT_TABLE[apt_indx];
    double ss = 1 / ((C_TABLE[c_indx] * pow(apt, 2)) / (lux * ISO_TABLE[iso_indx]));
    Serial.print("Apt: ");
    Serial.println(apt);
    Serial.print("calc speed: ");
    Serial.println(ss);

    // search for closest shutter speed
    for (int i = 0; i < ss_tbl_sz - 1; ++i) {
      if (ss >= SS_TABLE[i] && ss <= SS_TABLE[i+1]) {
        if (abs(ss - SS_TABLE[i]) <= abs(ss - SS_TABLE[i+1])) {
          ss_indx = i;
        } else {
          ss_indx = i + 1;
        }
      }
    }

    Serial.print("Shutter speed: 1/");
    Serial.println(SS_TABLE[ss_indx]);
  } else {
    // calculate APT 
    double ss = 1 / SS_TABLE[ss_indx];
    double apt = sqrt(((lux * ISO_TABLE[iso_indx]) / C_TABLE[c_indx]) * ss);
    Serial.print("Calc apt: ");
    Serial.println(apt);

    // search for closest apt 
    for (int i = 0; i < apt_tbl_sz - 1; ++i) {
      if (apt >= APT_TABLE[i] && apt <= APT_TABLE[i+1]) {
        if (abs(apt - APT_TABLE[i]) <= abs(apt - APT_TABLE[i+1])) {
          apt_indx = i;
        } else {
          apt_indx = i + 1;
        }
      }
    }

    Serial.print("apt: ");
    Serial.println(APT_TABLE[apt_indx]);
  }

  // calculate ev detla
  double actual_ev = log2((lux * ISO_TABLE[iso_indx]) / C_TABLE[c_indx]);
  double calc_ev = log2(pow(APT_TABLE[apt_indx], 2.0) / (1.0 / SS_TABLE[ss_indx]));

  ev_delta = actual_ev - calc_ev;
}

void display_info() {
  if (selected_mode == MODE_SETTINGS_EDIT || selected_mode == MODE_APT_EDIT || selected_mode == MODE_SS_EDIT) {
    display_editing_screen();
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  selected_mode == MODE_SETTINGS ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  display.println("SETTINGS");

  selected_mode == MODE_SS ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_left_1x(String("1/") + String(int(SS_TABLE[ss_indx])) + String((selected_prio == SS_PRIO ? " P" : "")));

  selected_mode == MODE_APT ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_right_1x(String("f ") + String(APT_TABLE[apt_indx]) + String((selected_prio == APT_PRIO ? " P" : "")));
  display.println();

  display.setTextColor(SSD1306_WHITE);
  display.print("EV delta: ");
  display.println(ev_delta);

  display.display();
}

// when the user is editing a value
void display_editing_screen() {

  if (selected_mode == MODE_SETTINGS_EDIT) {
    show_settings();
    return;
  }

  display.clearDisplay();
  display.setTextSize(2); // 2x font for readability
  display.setCursor(0,0);

  display.setTextColor(SSD1306_WHITE);


  if (selected_mode == MODE_SS_EDIT) {
    display.println("SS");
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.print("1/");
    display.println(int(SS_TABLE[ss_indx]));
  }

  if (selected_mode == MODE_APT_EDIT) {
    display.println("APETURE");
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.print("f ");
    display.println(APT_TABLE[apt_indx]);
  }

  display.display();
}
