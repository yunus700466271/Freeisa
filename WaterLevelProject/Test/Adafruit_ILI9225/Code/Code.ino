#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_ILI9225.h>

// Replace with your network credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

WebServer server(80);

// Define LCD pins
#define TFT_CS 15
#define TFT_RST -1
#define TFT_DC 2
#define TFT_MOSI 23
#define TFT_CLK 18

Adafruit_ILI9225 tft = Adafruit_ILI9225(TFT_CS, TFT_DC, TFT_RST);

// Define sensor pins
#define TRIGGER_PIN 13
#define ECHO_PIN 12

void setup() {
  Serial.begin(115200);

  // Initialize LCD
  tft.begin();
  tft.setRotation(1);

  // Initialize sensor
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Connect to Wi-Fi network
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Print ESP32 IP address
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Handle root URL
  server.on("/", [](){
    // Trigger sensor and measure distance
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
    float duration = pulseIn(ECHO_PIN, HIGH);
    float distance = duration * 0.034 / 2;

    // Display distance on LCD
    tft.fillScreen(ILI9225_WHITE);
    tft.setCursor(50, 50);
    tft.setTextColor(ILI9225_BLACK);
    tft.setTextSize(2);
    tft.println(String(distance) + " cm");

    // Send distance to web page
    String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><style>body { font-family: Arial, sans-serif; margin: 0; } h1 { text-align: center; margin-top: 50px; } p { text-align: center; font-size: 30px; margin-top: 50px; }</style></head><body><h1>Distance Measurement</h1><p>";
    html += String(distance);
    html += " cm</p></body></html>";
    server.send(200, "text/html", html);
  });

  // Start server
  server.begin();
  Serial.println("Server started");
}

void loop() {
  server.handleClient();
}
