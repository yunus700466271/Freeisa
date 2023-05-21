#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Ultrasonic.h>
#include <TFT_22_ILI9225.h>


// Replace with your network credentials
const char* ssid = "Your_SSID";
const char* password = "Your_PASSWORD";

// Create an instance of the server
WebServer server(80);

// Define ultrasonic sensor pins
#define TRIG_PIN 23
#define ECHO_PIN 22

// Create an instance of the Ultrasonic sensor
Ultrasonic sensor(TRIG_PIN, ECHO_PIN);

// Create an instance of the LCD display
TFT_22_ILI9225 tft = TFT_22_ILI9225(26, 27, 25, 17, 16);

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Connect to Wi-Fi network
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  // Print the IP address of the access point
  Serial.print("Access Point IP address: ");
  Serial.println(WiFi.softAPIP());

  // Initialize the LCD display
  tft.begin();
  //tft.setRotation(1);
  tft.fillScreen(ILI9225_BLACK);
 
  // Serve the HTML and CSS files
  server.on("/", [](){
    float distance = sensor.read() / 10.0; // read distance in cm and convert to mm
    char distanceStr[10];
    sprintf(distanceStr, "%.1f", distance);
    tft.fillRect(0, 0, tft.width(), tft.height(), ILI9225_BLACK);
    tft.setTextColor(ILI9225_WHITE);
    tft.setFont();
    tft.setTextSize(2);
    tft.setCursor(20, 30);
    tft.print(distanceStr);
    tft.setTextSize(1);
    tft.setCursor(70, 5);
    tft.print("Distance:");
    tft.setCursor(120, 40);
    tft.print("mm");
    server.send(200, "text/html", "<html><head><style>body {background-color: #F5F5DC;}h1 {color: blue;}p {color: black;}</style></head><body><h1>Distance Measurement</h1><p>Distance: " + String(distance) + " mm</p></body></html>");
  });

  // Start the server
  server.begin();
}

void loop() {
  // Handle client requests
  server.handleClient();
}
