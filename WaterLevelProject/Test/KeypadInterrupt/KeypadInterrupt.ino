#include <Keypad.h>

// Define the keypad matrix
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {3, 21, 19, 18}; // Connect keypad row pins to these GPIO pins
byte colPins[COLS] = {5, 17, 16, 4}; // Connect keypad column pins to these GPIO pins


// Define variables to store keypad state
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
volatile boolean keyPressed = false;
volatile char key;


void IRAM_ATTR keyISR() {
  Serial.println("Entered");
  delay(50); // Debounce delay
  key = keypad.getKey();
  if (key != NO_KEY) {
    keyPressed = true;
  }
}



void setup() {
  // Set row pins as INPUT_PULLUP
  for (int i = 0; i < ROWS; i++) {
    pinMode(rowPins[i], INPUT_PULLUP);
  }
  
  // Set column pins as OUTPUT
  for (int i = 0; i < COLS; i++) {
    pinMode(colPins[i], OUTPUT);
    digitalWrite(colPins[i], LOW); // Set column pins HIGH to start
  }
  
  // Attach interrupt to detect when a key is pressed
  attachInterrupt(digitalPinToInterrupt(rowPins[0]), keyISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(rowPins[1]), keyISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(rowPins[2]), keyISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(rowPins[3]), keyISR, FALLING);
  
  Serial.begin(115200);
}

void loop() {
  if (keyPressed) {
    Serial.println(key); // Print the key that was pressed
    keyPressed = false; // Reset the flag
  }
}

// Interrupt service routine to detect when a key is pressed
