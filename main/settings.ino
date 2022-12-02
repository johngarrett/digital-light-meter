enum settings_mode { S_MODE_ISO, S_MODE_ISO_EDIT, S_MODE_FL, S_MODE_FL_EDIT, S_MODE_CVAL, S_MODE_CVAL_EDIT, S_MODE_SN, S_MODE_SN_EDIT, S_MODE_BACK };
settings_mode selected_s_mode = S_MODE_ISO;

boolean on_main_settings_screen() {
  return selected_s_mode == S_MODE_ISO || selected_s_mode == S_MODE_FL || selected_s_mode == S_MODE_CVAL || selected_s_mode == S_MODE_BACK || selected_s_mode == S_MODE_SN;
}

boolean on_edit_settings_screen() {
  return selected_s_mode == S_MODE_ISO_EDIT || selected_s_mode == S_MODE_FL_EDIT || selected_s_mode == S_MODE_CVAL_EDIT || selected_s_mode == S_MODE_SN_EDIT;
}

void handle_settings_input() {
  if (sel_state == 1) {
    switch (selected_s_mode) {
      case S_MODE_ISO:
        selected_s_mode = S_MODE_ISO_EDIT;
        break;
      case S_MODE_ISO_EDIT:
        selected_s_mode = S_MODE_ISO;
        break;
      case S_MODE_FL:
        selected_s_mode = S_MODE_FL_EDIT;
        break;
      case S_MODE_FL_EDIT:
        selected_s_mode = S_MODE_FL;
        break;
      case S_MODE_CVAL:
        selected_s_mode = S_MODE_CVAL_EDIT;
        break;
      case S_MODE_CVAL_EDIT:
        selected_s_mode = S_MODE_CVAL;
        break;
      case S_MODE_SN:
        selected_s_mode = S_MODE_SN_EDIT;
        break;
      case S_MODE_SN_EDIT:
        selected_s_mode = S_MODE_SN;
        break;
      case S_MODE_BACK:
        selected_s_mode = S_MODE_ISO; // reset
        selected_mode = MODE_SETTINGS;
        break;
    }
  delay(175); // prevent jittering
  }

  // navigate main screen
  if (on_main_settings_screen()) {
    switch (map(pot_val, 0, 1023, 1, 15)) {
      case 1:
      case 2:
      case 3:
        selected_s_mode = S_MODE_BACK;
        break;
      case 4:
      case 5:
      case 6:
        selected_s_mode = S_MODE_ISO;
        break;
      case 7:
      case 8:
      case 9:
        selected_s_mode = S_MODE_FL;
        break;
      case 10:
      case 11:
      case 12:
        selected_s_mode = S_MODE_CVAL;
        break;
      case 13:
      case 14:
      case 15:
        selected_s_mode = S_MODE_SN;
        break;
    }
  }

  if (selected_s_mode == S_MODE_ISO_EDIT) {
    iso_indx = map(pot_val, 0, 1023, 0, iso_tbl_sz);
    // for some reason, it glitches out at high values; this helps
    if (iso_indx == iso_tbl_sz) {
      iso_indx--;
    }
  }

  if (selected_s_mode == S_MODE_FL_EDIT) {
    fl_indx = map(pot_val, 0, 1023, 0, fl_tbl_sz);
    // for some reason, it glitches out at high values; this helps
    if (fl_indx == fl_tbl_sz) {
      fl_indx;
    }
  }

  if (selected_s_mode == S_MODE_CVAL_EDIT) {
    c_indx = map(pot_val, 0, 1023, 0, c_tbl_sz);
    // for some reason, it glitches out at high values; this helps
    if (c_indx == c_tbl_sz) {
      c_indx--;
    }
  }

  if (selected_s_mode == S_MODE_SN_EDIT) {
    shot_number = map(pot_val, 0, 1023, 0, 37);
    // for some reason, it glitches out at high values; this helps
    if (shot_number  == 37) {
      shot_number--;
    }
  }
}

void show_settings() {
  if (on_edit_settings_screen()) {
    display_settings_edit();
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  selected_s_mode == S_MODE_BACK ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_center_1x("BACK");
  display.println();

  selected_s_mode == S_MODE_ISO ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_left_1x(String("ISO  ") + String(ISO_TABLE[iso_indx]));

  selected_s_mode == S_MODE_FL ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_right_1x(String("F-len ") + String(FL_TABLE[fl_indx]) + String("mm"));
  display.println();

  selected_s_mode == S_MODE_CVAL ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_left_1x(String("C-val ") + String(C_TABLE[c_indx]));

  selected_s_mode == S_MODE_SN ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_right_1x(String("shot # ") + String(shot_number));
  display.println();


  display.display();
}

void display_settings_edit() {
  switch (selected_s_mode) {
    case S_MODE_ISO_EDIT:
      render_edit_screen("ISO", String(ISO_TABLE[iso_indx]));
      break;
    case S_MODE_FL_EDIT:
      render_edit_screen("F-length", String(FL_TABLE[fl_indx]));
      break;
    case S_MODE_CVAL_EDIT:
      render_edit_screen("C-Val", String(C_TABLE[c_indx]));
      break;
    case S_MODE_SN_EDIT:
      render_edit_screen("Shot Num", String(shot_number));
      break;
  }
}
