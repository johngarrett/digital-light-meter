enum history_mode { H_MODE_SCROLL, H_MODE_VIEW_ENTRY };
history_mode selected_h_mode = H_MODE_SCROLL;

bool has_read_history_files = false;
bool has_read_history_entry = false;
int history_file_start = 0;  // what file to start printing at
int history_entry_line_start = 0; // what line to start showing entry data at
int num_history_files = 0;
int num_entry_lines = 0;
String history_file_names[50];
String history_entry_lines[15];

void handle_history_input() {
  if (sel_state == 1) {
    switch (selected_h_mode) {
      case H_MODE_SCROLL:
        selected_h_mode = H_MODE_VIEW_ENTRY;
        break;
    }
  delay(175); // prevent jittering
  }

  // this acts as our back button
  if (rec_state == 1) {
    switch (selected_h_mode) {
      case H_MODE_SCROLL:
        selected_mode = MODE_HISTORY;
      case H_MODE_VIEW_ENTRY:
        selected_h_mode = H_MODE_SCROLL;
        has_read_history_entry = false; // next select will read a new file
    }
    delay(175);
  }

  if (selected_h_mode == H_MODE_SCROLL) {
    history_file_start = map(pot_val, 0, 1023, 0, num_history_files);
  }

  if (selected_h_mode == H_MODE_VIEW_ENTRY) {
    history_entry_line_start = map(pot_val, 0, 1023, 0, num_entry_lines);
  }
}

void read_history_files() {
  int itr = 0;
  File dir = SD.open("/SHOTS");
  for (File entry = dir.openNextFile(); entry.name()[0] != 0; entry = dir.openNextFile()) {
    history_file_names[itr] = build_file_string(entry, false);
    num_history_files++;
    itr++;
    // TODO: recursive search
    if (entry.isDirectory()) {
      for (File entry_child = entry.openNextFile(); entry_child.name()[0] != 0; entry_child = entry.openNextFile()) {
        history_file_names[itr] = build_file_string(entry_child, true);
        num_history_files++;
        itr++;
        entry_child.close();
      }
    }
    entry.close();
  }
  dir.close();
  has_read_history_files = true;
}

void read_history_entry() {
  File entry = SD.open(String("/SHOTS/") + history_file_start + String(".TXT"));

  int itr = 0;
  String line = "";
  while(entry.available()) {
    char character = entry.read();

    if (character == '\n') {
      history_entry_lines[itr] = String(line);
      itr++;
      line = String("");
      num_entry_lines++;
    } else {
      line += character;
    }
  }
  entry.close();

  has_read_history_entry = true;
}

void show_history() {
  if (!has_read_history_files) {
    read_history_files();
  }

  if (selected_h_mode == H_MODE_VIEW_ENTRY) {
    show_history_entry();
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  display.setTextColor(SSD1306_WHITE);
  print_center_1x("Press REC to go back");
  display.println();
  display.drawFastHLine(0, display.getCursorY(), display.width(), SSD1306_WHITE);

  if (!sd_available) {
    print_center_2x("NO SD Card");
    display.display();
    return;
  }

  display.setTextColor(SSD1306_WHITE);

  // we can only display 3 files at a time
  for (int offset = 0; offset < 3; ++offset) {
    offset == 0 ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
    display.println(history_file_names[history_file_start + offset]);
  }

  display.display();
}

void show_history_entry() {
  if (!has_read_history_entry) {
    read_history_entry();
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);

  display.setTextColor(SSD1306_WHITE);
  print_center_1x("Press REC to go back");
  display.println();
  display.drawFastHLine(0, display.getCursorY(), display.width(), SSD1306_WHITE);

  for (int offset = 0; offset < 3; ++offset) {
    offset == 0 ? display.setTextColor(SSD1306_BLACK, SSD1306_WHITE) : display.setTextColor(SSD1306_WHITE);
    display.println(history_entry_lines[history_entry_line_start + offset]);
  }

  display.display();
}
