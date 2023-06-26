#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <i2cdetect.h>
#include <math.h> 
#include "libs/BH1750/src/BH1750.h"
#include "memorysaver.h"
#include "include.h"
#include <SD.h>

#include <ArduCAM.h>
#include <SPI.h>

#define MAX_BAT_VOLT    4.2  // max voltage for lipo battery
#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   32
#define OLED_RESET      -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS  0x3C // 0x3D for 128x64, 0x3C for 128x32

#define SD_CS   4
#define SPI_CS  12
//#define REC_PIN 6
#define POT_PIN A1
#define BAT_PIN A7

// pins for the rotary encoder
#define ROT_CLK 5
#define ROT_DT  6
#define ROT_SW  9

#define MAX_ROT_VAL 20

const double APT_TABLE[]  = { 1.0, 1.4, 1.8, 2.0, 2.8, 3.5, 4.0, 4.5, 5.6, 6.3, 8.0, 11.0, 12.7, 16.0, 22.0, 32.0 };
const int ISO_TABLE[]     = { 6, 12, 25, 50, 100, 160, 200, 400, 800, 1600, 3200, 6400 };
const double SS_TABLE[]   = { -1, 1, 2, 4, 8, 15, 30, 60, 125, 250, 500, 1000 };
const int C_TABLE[]       = { 250, 330 }; // incident constant
const int FL_TABLE[]      = { 28, 50 }; // focal length

const int apt_tbl_sz = sizeof(APT_TABLE) / sizeof(APT_TABLE[0]);
const int iso_tbl_sz = sizeof(ISO_TABLE) / sizeof(ISO_TABLE[0]);
const int ss_tbl_sz = sizeof(SS_TABLE) / sizeof(SS_TABLE[0]);
const int c_tbl_sz = sizeof(C_TABLE) / sizeof(C_TABLE[0]);
const int fl_tbl_sz = sizeof(FL_TABLE) / sizeof(FL_TABLE[0]);

enum mode { MODE_BOOTUP, MODE_SS, MODE_SS_EDIT, MODE_APT, MODE_APT_EDIT, MODE_SETTINGS, MODE_SETTINGS_EDIT, MODE_HISTORY, MODE_HISTORY_EDIT, MODE_RECORD };
enum priority { APT_PRIO, SS_PRIO };

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
BH1750 lightMeter;

long lux;
double actual_ev, calc_ev, ev_delta = 0;
int apt_indx, iso_indx, ss_indx, fl_indx, c_indx = 0;
int shot_number = 0;
int sel_state = 0;
int rec_state = 0;
int pot_val = 0;
float bat_val = 0;
int recorded_ss_indx, recorded_apt_indx = 0;
bool sd_available = false;

mode selected_mode = MODE_BOOTUP;
priority selected_prio = APT_PRIO;

void setup() {
  Serial.begin(9600);
  //while (!Serial) { }
  Wire.begin();

  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire)) {
    Serial.println(F("Light meter initialized"));
  } else {
    Serial.println(F("Error initing lightmeter"));
  }

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  /*
  sd_available = SD.begin(SD_CS);

  if (!sd_available) {
    Serial.println(F("SD Card failed to initialize!"));
  }
  */

  //pinMode(REC_PIN, INPUT);
  pinMode(ROT_CLK, INPUT);
  pinMode(ROT_DT,  INPUT);
  pinMode(ROT_SW,  INPUT_PULLUP);

  // read values from SD card for APT, ISO, SS, etc.
  //read_stored_values();
}

// display ISO, F-length, and shot number on bootup
int bootup_countdown = 75;
void display_bootup_screen() {
  // the default mode is MODE_BOOTUP, keep it that way until bootup_countdown reaches 0
  bootup_countdown--;

  if (bootup_countdown == 0) {
    selected_mode = MODE_SETTINGS;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  display.setTextColor(SSD1306_WHITE);

  print_left_1x(String("ISO ") + String(ISO_TABLE[iso_indx]));
  print_right_1x(String("F-Len ") + String(FL_TABLE[fl_indx]));
  display.println();
  print_center_2x(String("Shot #") + String(shot_number));
  display.println('\n');

  // drawing battery percentage

  display.print(int((bat_val / MAX_BAT_VOLT) * 100));
  display.print("% ");
  int x = display.getCursorX();
  int y = display.getCursorY();
  int bar_width = (bat_val / MAX_BAT_VOLT) * (display.width() - x);

  for (int yoff = 0; yoff < 6; ++yoff) {
    display.drawFastHLine(x, y + yoff, bar_width, SSD1306_WHITE);
  }

  display.display();
}

void loop() {
  read_inputs();
  handle_inputs();
  read_lux();
  calculate_stats();
  display_info();
}

int prev_rclk = 0;
void read_inputs() {
  sel_state = digitalRead(ROT_SW) == 0 ? 1 : 0;
  rec_state = 0; //digitalRead(REC_PIN);
  //Serial.print("sel: ");
  //Serial.println(sel_state);

  // rotary encoder
  int curr_rclk = digitalRead(ROT_CLK);
  
  if (curr_rclk != prev_rclk && curr_rclk == 1) {
    // CW vs CCW
    if (digitalRead(ROT_DT) != curr_rclk) {
      pot_val--;
      Serial.println("CCW");
    } else {
      pot_val++;
      Serial.println("CW");
    }
    //pot_val = (digitalRead(ROT_DT) == curr_rclk) ? pot_val + 1 : pot_val - 1;
    // 20 notches on rotary encoder
    //pot_val %= 20;
    Serial.println(pot_val);
  }
  prev_rclk = curr_rclk;

  // battery value calculation 
  // https://learn.adafruit.com/adafruit-feather-m0-adalogger/power-management
  //bat_val = analogRead(BAT_PIN);
  bat_val *= 2;
  bat_val *= 3.3; // reference voltage
  bat_val /= 1024; // convert to voltage
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
    capture_image();
    delay(175);
  }

  // switching between parent and child view
  if (sel_state == 1) {
    switch (selected_mode) {
      case MODE_APT:
        selected_mode = MODE_APT_EDIT;
        break;
      case MODE_APT_EDIT:
        update_stored_info();
        selected_mode = MODE_APT;
        break;
      case MODE_SS:
        selected_mode = MODE_SS_EDIT;
        break;
      case MODE_SS_EDIT:
        update_stored_info();
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
    //switch (map(pot_val, 0, MAX_ROT_VAL, 1, 12)) {
   // Serial.println(pot_val % 4);
    switch (pot_val % 4) {
      case 0:
        selected_mode = MODE_SETTINGS;
        break;
      case 1:
        selected_mode = MODE_HISTORY;
        break;
      case 2:
        // ordering changes based on priority
        selected_mode = (selected_prio == APT_PRIO) ? MODE_SS : MODE_APT;
        break;
      case 3:
        selected_mode = (selected_prio == SS_PRIO) ? MODE_SS : MODE_APT;
        break;
    }
  }

  if (selected_mode == MODE_SS_EDIT) {
    selected_prio = SS_PRIO;
    ss_indx = map(pot_val, 0, MAX_ROT_VAL, 0, ss_tbl_sz);
    if (ss_indx  == ss_tbl_sz) {
      ss_indx--;
    }
  }

  if (selected_mode == MODE_APT_EDIT) {
    selected_prio = APT_PRIO;
    apt_indx = map(pot_val, 0, MAX_ROT_VAL, 0, apt_tbl_sz);
    if (apt_indx == apt_tbl_sz) {
      apt_indx--;
    }
  }
}

void read_lux() {
  lux = lightMeter.readLightLevel();
  if (lux == -1) {
    lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
  }
}

// calculate necessary SS, APT based on prio and lux
void calculate_stats() {
  actual_ev = log2((lux * ISO_TABLE[iso_indx]) / C_TABLE[c_indx]);
  /*
    N^2 / t = ES / C

    N = apeture
    t = shutter speed (in seconds)
    E = illuminance
    S = ISO arithmetic speed
    C = incident-light constant
   */
  if (selected_prio == APT_PRIO) {
    /*
       t = (N^2 * C) / ES
     */


    // calculate SS 
    double apt = APT_TABLE[apt_indx];
    double ss = pow(2.0, actual_ev) / pow(apt, 2.0);

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
    double ss = SS_TABLE[ss_indx];
    double apt = sqrt(pow(2.0, actual_ev) / ss);

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
  calc_ev = log2(pow(APT_TABLE[apt_indx], 2.0) / (1.0 / SS_TABLE[ss_indx]));

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

  if (selected_mode == MODE_BOOTUP) {
    display_bootup_screen();
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  selected_mode == MODE_SETTINGS ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  display.print("S");
  display.setTextColor(SSD1306_WHITE);
  display.print(" ");
  selected_mode == MODE_HISTORY ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  display.print("H");

  display.setTextColor(SSD1306_WHITE);
  print_right_1x(int(lux) + String("lx ") + int((bat_val / MAX_BAT_VOLT) * 100));


  display.println();
  display.drawFastHLine(0, display.getCursorY(), display.width(), SSD1306_WHITE);

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
