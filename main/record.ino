#include <SPI.h> 

enum recording_mode { R_MODE_CANCEL, R_MODE_SAVE };
recording_mode selected_r_mode = R_MODE_SAVE;


void save_recording() {
  shot_number++;
}

void handle_record_input() {
  if (sel_state == 1) {
    switch (selected_r_mode) {
      case R_MODE_CANCEL:
        selected_mode = MODE_SETTINGS;
        break;
      case R_MODE_SAVE:
        save_recording();
        selected_mode = MODE_SETTINGS;
        break;
    }
    delay(175);
  }

  switch (map(pot_val, 0, 1023, 1, 6)) {
    case 1:
    case 2:
    case 3:
      selected_r_mode = R_MODE_CANCEL;
      break;
    case 4:
    case 5:
    case 6:
      selected_r_mode = R_MODE_SAVE;
      break;
  }
}

// display stats to be recorded
void show_recording() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  display.setTextColor(SSD1306_WHITE);

  if (selected_prio == SS_PRIO) {
    print_center_2x(String("f ") + String(APT_TABLE[recorded_apt_indx]) + String((selected_prio == APT_PRIO ? " P" : "")));
    display.println("\n"); // two new lines

    print_center_1x(String("1/") + String(int(SS_TABLE[recorded_ss_indx])) + String((selected_prio == SS_PRIO ? " P" : "")));
  } else {
    print_center_2x(String("1/") + String(int(SS_TABLE[recorded_ss_indx])) + String((selected_prio == SS_PRIO ? " P" : "")));
    display.println("\n"); // two new lines

    print_center_1x(String("f ") + String(APT_TABLE[recorded_apt_indx]) + String((selected_prio == APT_PRIO ? " P" : "")));
  }

  display.println();
  
  // TODO: show shot number?

  selected_r_mode == R_MODE_CANCEL ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_left_1x("Cancel");

  selected_r_mode == R_MODE_SAVE ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_right_1x("Save");

  display.println();

  display.display();
}
