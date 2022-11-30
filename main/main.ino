#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <i2cdetect.h>
#include <math.h> 
#include "src/BH1750/src/BH1750.h"
#include <SD.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C /// 0x3D for 128x64, 0x3C for 128x32
#define SEL_PIN  5
#define REC_PIN  6
#define POT_PIN   A5
#define SD_CS   4

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

enum mode { MODE_SS, MODE_SS_EDIT, MODE_APT, MODE_APT_EDIT, MODE_SETTINGS, MODE_SETTINGS_EDIT, MODE_HISTORY, MODE_HISTORY_EDIT, MODE_RECORD };
enum priority { APT_PRIO, SS_PRIO };

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

BH1750 lightMeter;

float lux;
double ev_delta = 0;
int apt_indx, iso_indx, ss_indx, c_indx, fl_indx = 0;
int shot_number = 0;
int sel_state = 0;
int rec_state = 0;
int pot_val = 0;
int recorded_ss_indx, recorded_apt_indx = 0;

mode selected_mode = MODE_SETTINGS;
priority selected_prio = APT_PRIO;

void setup() {
  Serial.begin(9600);
  //while (!Serial) { }
  Wire.begin();
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire)) {
    Serial.println(F("Began"));
  } else {
    Serial.println(F("Error initing lightmeter"));
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  if (!SD.begin(SD_CS)) {
    Serial.println(F("SD Card failed to initialize!"));
  }


  pinMode(SEL_PIN, INPUT);
  pinMode(REC_PIN, INPUT);

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

// create fs if DNE, read values from files if it does
void initialize_fs() {
  /*

    /info:
      ISO ####
      SHOT ##
      CVAL_IDX ##
      F_LEN_IDX ##
    /last_vals:
      PRIO #
      APT_IDX ##
      SS_IDX ##
   */
  File info; 

  if (SD.exists("/info")) {
    Serial.println(F("/info exists, reading from it"));
    info = SD.open("/info", FILE_READ);
    // TODO
  } else {
    Serial.println(F("/info DNE, creating it"));
    info = SD.open("/info", FILE_WRITE);
    // TODO
  }

  info.close();



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
  rec_state = digitalRead(REC_PIN);
  pot_val = analogRead(POT_PIN);
}

boolean on_main_screen() {
  return selected_mode == MODE_SETTINGS || selected_mode == MODE_HISTORY || selected_mode == MODE_SS || selected_mode == MODE_APT;
}

boolean on_edit_screen() {
  return selected_mode == MODE_SETTINGS_EDIT || selected_mode == MODE_HISTORY_EDIT || selected_mode == MODE_APT_EDIT || selected_mode == MODE_SS_EDIT;
}

void handle_inputs() {
  // pass control over to the settings file
  if (selected_mode == MODE_SETTINGS_EDIT) {
    handle_settings_input();
    return;
  }

  if (selected_mode == MODE_HISTORY_EDIT) {
    handle_history_input();
    return;
  }

  if (selected_mode == MODE_RECORD) {
    handle_record_input();
    return;
  }

  if (rec_state == 1) {
    selected_mode = MODE_RECORD;
    recorded_apt_indx = apt_indx;
    recorded_ss_indx = ss_indx;
    delay(175);
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
      case MODE_HISTORY:
        selected_mode = MODE_HISTORY_EDIT;
        break;
    }
    delay(175); // to prevent jittering
  }

  // the user wants to set a value to be automatically decided
  if (on_main_screen()) {
    // map pot val to { SET, HIST, SS, APT } (3x for better error handling)
    switch (map(pot_val, 0, 1023, 1, 12)) {
      case 1:
      case 2:
      case 3:
        selected_mode = MODE_SETTINGS;
        break;
      case 4:
      case 5:
      case 6:
        selected_mode = MODE_HISTORY;
        break;
      case 7:
      case 8:
      case 9:
        // ordering changes based on priority
        selected_mode = (selected_prio == APT_PRIO) ? MODE_SS : MODE_APT;
        break;
      case 10:
      case 11:
      case 12:
        selected_mode = (selected_prio == SS_PRIO) ? MODE_SS : MODE_APT;
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
  } else {
    // calculate APT 
    double ss = 1 / SS_TABLE[ss_indx];
    double apt = sqrt(((lux * ISO_TABLE[iso_indx]) / C_TABLE[c_indx]) * ss);

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
  }

  // calculate ev detla
  double actual_ev = log2((lux * ISO_TABLE[iso_indx]) / C_TABLE[c_indx]);
  double calc_ev = log2(pow(APT_TABLE[apt_indx], 2.0) / (1.0 / SS_TABLE[ss_indx]));

  ev_delta = actual_ev - calc_ev;
}

void display_info() {
  if (on_edit_screen()) {
    display_editing_screen();
    return;
  }

  if (selected_mode == MODE_RECORD) {
    show_recording();
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  selected_mode == MODE_SETTINGS ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_left_1x("SETTINGS");

  selected_mode == MODE_HISTORY ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_right_1x("HISTORY");
  display.println();

  if (selected_prio == SS_PRIO) {
    /*
        |    APT    |
        |    APT    |
        | ss     ev |
    */
    selected_mode == MODE_APT ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
    print_center_2x(String("f ") + String(APT_TABLE[apt_indx]) + String((selected_prio == APT_PRIO ? " P" : "")));
    display.println("\n"); // two new lines

    selected_mode == MODE_SS ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
    print_left_1x(String("1/") + String(int(SS_TABLE[ss_indx])) + String((selected_prio == SS_PRIO ? " P" : "")));

    display.setTextColor(SSD1306_WHITE);
    print_right_1x(String(ev_delta));
  } else {
    /*
        |    SS     |
        |    SS     |
        | apt    ev |
    */
    selected_mode == MODE_SS ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
    print_center_2x(String("1/") + String(int(SS_TABLE[ss_indx])) + String((selected_prio == SS_PRIO ? " P" : "")));
    display.println("\n"); // two new lines

    selected_mode == MODE_APT ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
    print_left_1x(String("f ") + String(APT_TABLE[apt_indx]) + String((selected_prio == APT_PRIO ? " P" : "")));

    display.setTextColor(SSD1306_WHITE);
    print_right_1x(String(ev_delta));
  }

  display.println();
  display.display();
}

// when the user is editing a menu item
void display_editing_screen() {
  switch (selected_mode) {
    case MODE_SETTINGS_EDIT:
      show_settings();
      break;
    case MODE_HISTORY_EDIT:
      show_history();
      break;
    case MODE_SS_EDIT:
      render_edit_screen("SS", String("1/") + String(SS_TABLE[ss_indx]));
      break;
    case MODE_APT_EDIT:
      render_edit_screen("APETURE", String("f ") + String(APT_TABLE[apt_indx]));
      break;
  }
}
