enum recording_mode { R_MODE_CANCEL, R_MODE_SAVE };
recording_mode selected_r_mode = R_MODE_SAVE;


void save_recording() {
  if (!sd_available) {
    Serial.println("Unable to save recording, SD not available");
    return;
  }

  if (!SD.exists("/shots")) {
    Serial.println(F("/shots DNE, creating"));
    if (!SD.mkdir("/shots")) {
      Serial.println(F("mkdir /shots FAILED"));
    }
  }

  String file_name = String("/shots/") + String(shot_number) + String(".txt");

  File file = SD.open(file_name.c_str(), O_WRITE | O_CREAT | O_TRUNC);

  file.print("ISO ");
  file.println(ISO_TABLE[iso_indx]);

  file.print("CVAL ");
  file.println(C_TABLE[c_indx]);

  file.print("F_LEN ");
  file.println(FL_TABLE[fl_indx]);

  file.print("PRIO ");
  file.println(selected_prio == APT_PRIO ? "APT" : "SS");

  file.print("APT ");
  file.println(APT_TABLE[apt_indx]);

  file.print("SS ");
  file.println((String("1/") + String(SS_TABLE[ss_indx])).c_str());

  file.print("LUX ");
  file.println(lux);

  file.print("EV_ACTUAL ");
  file.println(actual_ev);

  file.print("EV_CALC ");
  file.println(calc_ev);

  file.print("EV_DELTA ");
  file.println(ev_delta);

  file.close();
  shot_number++;
  update_stored_info();
}

/**
This is called the second record is pressed.

TODO: return enum of failure modes
*/
void capture_image() {
  if (!sd_available) {
    Serial.println("Unable to capture image, SD not available");
    return;
  }
  char file_name[8];
  char buf[256];
  static int i = 0;
  uint8_t temp = 0, temp_last = 0;
  uint32_t length = 0;
  bool is_header = false;
  File outFile;

  // TODO: handle duplicate file names
  itoa(shot_number, file_name, 10);
  strcat(file_name, ".jpg");
  outFile = SD.open(file_name, O_WRITE | O_CREAT | O_TRUNC);
  if(!outFile) {
    Serial.println(F("File open faild"));
    return;
  }
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

  switch (map(pot_val, 0, MAX_ROT_VAL, 1, 6)) {
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

  if (!sd_available) {
    print_center_2x("NO SD Card");
    display.println("\n"); // two new lines
  } else {
    if (selected_prio == SS_PRIO) {
      print_center_2x(String("f ") + String(APT_TABLE[recorded_apt_indx]) + String((selected_prio == APT_PRIO ? " P" : "")));
      display.println("\n"); // two new lines

      print_left_1x(String("1/") + String(int(SS_TABLE[recorded_ss_indx])) + String((selected_prio == SS_PRIO ? " P" : "")));
      print_right_1x(String("#") + String(shot_number));
    } else {
      print_center_2x(String("1/") + String(int(SS_TABLE[recorded_ss_indx])) + String((selected_prio == SS_PRIO ? " P" : "")));
      display.println("\n"); // two new lines

      print_left_1x(String("f ") + String(APT_TABLE[recorded_apt_indx]) + String((selected_prio == APT_PRIO ? " P" : "")));
      print_right_1x(String("#") + String(shot_number));
    }
    display.println();
  }

  selected_r_mode == R_MODE_CANCEL ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_left_1x("Cancel");

  selected_r_mode == R_MODE_SAVE ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_right_1x("Save");

  display.println();

  display.display();
}
