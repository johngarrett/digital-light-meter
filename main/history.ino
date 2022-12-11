enum history_mode { H_MODE_BACK }; // TODO: H_MODE_BACK SCROLL, H_MODE_EDIT_ENTRY?
history_mode selected_h_mode = H_MODE_BACK;

void handle_history_input() {
  if (sel_state == 1) {
    switch (selected_h_mode) {
      case H_MODE_BACK:
        selected_mode = MODE_HISTORY;
        break;
    }
  delay(175); // prevent jittering
  }
}

void show_history() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  selected_h_mode == H_MODE_BACK ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
  print_center_1x("BACK");
  display.println();

  if (!sd_available) {
    print_center_2x("NO SD Card");
    display.display();
    return;
  }

  display.setTextColor(SSD1306_WHITE);
  while (true) {
    File dir = SD.open("/SHOTS");
    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      dir.close();
      break;
    }

    display.print(entry.name());
    entry.close();
  }
  display.println("\n");

  display.display();
}


