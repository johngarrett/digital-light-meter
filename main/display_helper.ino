/**
  screen width == 21 chars (in 1x mode)
  left align: 10 chars on left
  right align: 10 chars on right
*/
// left aligned
void print_left_1x(String str) {
  int y = display.getCursorY();
  display.setCursor(0, y);
  display.print(str);
  for (int i = 10; i > str.length(); --i) {
    display.print(' ');
  }
}

// right aligned
void print_right_1x(String str) {
  int y = display.getCursorY();
  display.setCursor((display.width() / 2) + 1, y);
  for (int i = 10; i > str.length(); --i) {
    display.print(' ');
  }
  display.print(str);
}

// print full width
void print_full_1x(String str) {
  display.print(str);
  for (int i = 21; i > str.length(); --i) {
    display.print(' ');
  }
}
void print_center_1x(String str) {
  int offset = str.length() % 2 == 0 ? 0 : 1; // one side needs to be longer
  for (int i = 0; i < 20 / 2 - (str.length() / 2) - offset; ++i) {
    display.print(' ');
  }
  display.print(str);
  for (int i = 0; i < 20 / 2 - (str.length() / 2) + 1; ++i) {
    display.print(' ');
  }
}

/**
  screen width == 10 chars (in 2x mode)
  left align: 5 chars on left
  right align: 5 chars on right
*/

void print_center_2x(String str) {
  display.setTextSize(2);
  int y = display.getCursorY();
  int offset = str.length() % 2 == 0 ? 0 : 1; // one side needs to be longer
  for (int i = 0; i < 10 / 2 - (str.length() / 2) - offset; ++i) {
    display.print(' ');
  }
  display.print(str);
  for (int i = 0; i < 10 / 2 - (str.length() / 2); ++i) {
    display.print(' ');
  }
  display.setTextSize(1);
}

// show editing screen with `title` to edit `value`
void render_edit_screen(String title, String value) {
  display.clearDisplay();
  display.setTextSize(2); // 2x font for readability
  display.setCursor(0,0);

  display.setTextColor(SSD1306_WHITE);

  display.println(title);
  display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
  display.println(value);

  display.display();
}
