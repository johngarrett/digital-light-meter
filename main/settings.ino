
enum settings_mode { S_MODE_ISO, S_MODE_ISO_EDIT, S_MODE_BACK };
settings_mode selected_s_mode = S_MODE_ISO;

boolean on_main_settings_screen() {
  return selected_s_mode == S_MODE_ISO || selected_s_mode == S_MODE_BACK;
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
      case S_MODE_BACK:
        selected_s_mode = S_MODE_ISO; // reset
        selected_mode = MODE_SETTINGS;
        break;
    }
  delay(175); // prevent jittering
  }

  // navigate main screen
  if (on_main_settings_screen()) {
    switch (map(pot_val, 0, 1023, 1, 6)) {
      case 1:
      case 2:
      case 3:
        selected_s_mode = S_MODE_ISO;
        break;
      case 4:
      case 5:
      case 6:
        selected_s_mode = S_MODE_BACK;
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

}

void show_settings() {
  if (selected_s_mode == S_MODE_ISO_EDIT) {
    display_settings_edit();
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  selected_s_mode == S_MODE_ISO ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  display.print("ISO  ");
  display.println(ISO_TABLE[iso_indx]);

  display.setTextColor(SSD1306_WHITE);
  display.println("FOCAL 28mm");

  display.setTextColor(SSD1306_WHITE);
  display.println("C-Val 330");

  selected_s_mode == S_MODE_BACK ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  display.println("BACK");

  display.display();
}

void display_settings_edit() {
  if (selected_s_mode == S_MODE_ISO_EDIT) {
    display.clearDisplay();
    display.setTextSize(2); // 2x font for readability
    display.setCursor(0,0);

    display.setTextColor(SSD1306_WHITE);
    display.println("ISO");
    display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
    display.println(ISO_TABLE[iso_indx]);
    display.display();
    return;
  }
}
