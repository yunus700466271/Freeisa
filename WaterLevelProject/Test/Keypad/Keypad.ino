#include <Keypad.h>

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {3, 21, 19, 18}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins 23, 3, 21, 19
byte pin_column[COLUMN_NUM] = {5, 17, 16, 4};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins 18, 5, 17, 16

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

void setup() {
  Serial.begin(115200);
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.println(key);
  }
}
