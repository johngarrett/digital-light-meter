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
