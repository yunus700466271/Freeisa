#include <UTFT.h>

// Pin definitions
#define TRIGGER_PIN 23
#define ECHO_PIN 22
#define LCD_CS A5
#define LCD_CD A4
#define LCD_WR A3
#define LCD_RD A2
#define LCD_RESET A1

// Create an instance of the UTFT library
UTFT myGLCD(ILI9225, LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

// Variables for storing the distance and timer values
long duration;
int distance;

void setup() {
  // Set up the serial communication
  Serial.begin(9600);

  // Initialize the JSN-SR04T sensor pins
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize the ILI9225 TFT display
  myGLCD.InitLCD();
  myGLCD.clrScr();
  myGLCD.setFont(SmallFont);
  myGLCD.setColor(255, 255, 255);
  myGLCD.print("Distance: ", 10, 10);
}

void loop() {
  // Trigger the JSN-SR04T sensor by sending a 10us pulse on the trigger pin
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Read the duration of the pulse on the echo pin
  duration = pulseIn(ECHO_PIN, HIGH);

  // Calculate the distance in centimeters
  distance = duration * 0.034 / 2;

  // Print the distance to the serial monitor
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // Clear the previous distance value on the display and print the new value
  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect(100, 10, 200, 30);
  myGLCD.setColor(255, 255, 255);
  myGLCD.printNumI(distance, 100, 10);
 
  // Wait for 500ms before taking the next measurement
  delay(500);
}
