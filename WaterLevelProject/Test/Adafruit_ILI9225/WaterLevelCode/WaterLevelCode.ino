#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>

#define TRIG_PIN 14 // GPIO14
#define ECHO_PIN 12 // GPIO12
#define I2C_ADDR 0x3F // I2C address of LCD module
#define LCD_ROWS 2 // number of rows on LCD
#define LCD_COLS 16 // number of columns on LCD

const char* ssid = "ESP32_AP";
const char* password = "password";
WebServer server(80);

LiquidCrystal_I2C lcd(I2C_ADDR, LCD_COLS, LCD_ROWS);
long distance = 0;

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Distance:");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");
  Serial.begin(115200);
  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  long duration, cm;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  cm = duration / 58;
  distance = cm;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Distance:");
  lcd.setCursor(0, 1);
  lcd.print(distance);
  server.handleClient();
}

void handleRoot() {
  String html = "<html><head><style>body{font-family: Arial, Helvetica, sans-serif;}.distance{font-size: 48px;}</style></head><body><h1>Distance Measurement</h1><p class='distance'>";
  html += distance;
  html += " cm</p></body></html>";
  server.send(200, "text/html", html);
}
