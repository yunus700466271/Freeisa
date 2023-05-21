#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include <../fonts/FreeSans12pt7b.h>
#include <../fonts/FreeSans9pt7b.h>


#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "MyESP32AP"; // change to your desired access point name
const char* password = "password"; // change to your desired access point password

const int trigPin = 22; // change to the actual pin number that the JSN-SR04T trigger pin is connected to
const int echoPin = 23; // change to the actual pin number that the JSN-SR04T echo pin is connected to

WebServer server(80);




#define TFT_RST 26  // IO 26
#define TFT_RS  25  // IO 25
#define TFT_CLK 14  // HSPI-SCK
// #define TFT_SDO 12  // HSPI-MISO
#define TFT_SDI 13  // HSPI-MOSI
#define TFT_CS  15  // HSPI-SS0
#define TFT_LED 0   // 0 if wired to +5V directly

SPIClass hspi(HSPI);

#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)


// Use hardware SPI (faster - on Uno: 13-SCK, 12-MISO, 11-MOSI)
TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
// Use software SPI (slower)
//TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_SDI, TFT_CLK, TFT_LED, TFT_BRIGHTNESS);

// Variables and constants
int16_t x=0, y=20, w, h;
String distanceStr,amountstr,storagestr; // Create string object
long duration, distance , distanceTemp  , amount ;
float tankHight = 2270,storage;


// Setup
void setup() {

  Serial.begin(115200);

  // initialize the WiFi access point
  WiFi.softAP(ssid, password);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // start the web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");



  
#if defined(ESP32)
  hspi.begin();
  tft.begin(hspi);
#else
  tft.begin();
#endif
  tft.clear();



  tft.setGFXFont(&FreeSans12pt7b); // Set current font
  tft.getGFXTextExtent("Fluid Level", x, y, &w, &h); // Get string extents
  y += h; // Set y position to string height
  x += 20;
  tft.drawGFXText(x, y, "Fluid Level", COLOR_CYAN); // Print string


  
  tft.setGFXFont(&FreeSans9pt7b);  // Change font
  tft.getGFXTextExtent(" Distance", x, y, &w, &h); // Get string extents
  y += h + 33; // Set y position to string height plus shift down 10 pixels
  x+=20;
  tft.drawGFXText(x, y, " Distance", COLOR_GREEN); // Print string
  
  
  
  tft.setGFXFont(&FreeSans9pt7b);  // Change font
  tft.getGFXTextExtent(" Amount", x, y, &w, &h); // Get string extents
  y += h + 33; // Set y position to string height plus shift down 10 pixels
  tft.drawGFXText(x, y, " Amount", COLOR_GREEN); // Print string
  
  
  
  tft.setGFXFont(&FreeSans9pt7b);  // Change font
  tft.getGFXTextExtent(" Storage", x, y, &w, &h); // Get string extents
  y += h + 33; // Set y position to string height plus shift down 10 pixels
  tft.drawGFXText(x, y, " Storage", COLOR_GREEN); // Print string

}


// Loop
void loop() {

  x=0;y=60;
  
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = tankHight-duration * 0.34 / 2;
  amount = 1.3749 * distance + 0.3357994;
  storage = 100*(distance/tankHight);


  distanceStr = String(distance) + " mm";
  amountstr =String(amount) + " Liters";
  storagestr = String(storage) + "%";
  
  server.handleClient();


  delay(500);
  if(int(distance)!=int(distanceTemp)){

    

    tft.setGFXFont(&FreeSans9pt7b);  // Change font
    tft.getGFXTextExtent(distanceStr, x, y, &w, &h); // Get string extents
    y += h + 33; // Set y position to string height plus shift down 10 pixels
    x+= 55;
    // Draw solid rectangular to clear the string
    tft.fillRectangle(40, 90, 140, 110, COLOR_BLACK); //(x1,y1,x2,y2,color)
    
    tft.drawGFXText(x, y, distanceStr, COLOR_WHITE); // Print string




    tft.setGFXFont(&FreeSans9pt7b);  // Change font
    tft.getGFXTextExtent(amountstr, x, y, &w, &h); // Get string extents
    y += h + 33; // Set y position to string height plus shift down 10 pixels
    x-=15;
    // Draw solid rectangular to clear the string
    tft.fillRectangle(40, 90+45, 140, 110+45, COLOR_BLACK); //(x1,y1,x2,y2,color)
    
    tft.drawGFXText(x, y, amountstr, COLOR_WHITE); // Print string


    tft.setGFXFont(&FreeSans9pt7b);  // Change font
    tft.getGFXTextExtent(storagestr, x, y, &w, &h); // Get string extents
    y += h + 33; // Set y position to string height plus shift down 10 pixels
    x+=15;
    // Draw solid rectangular to clear the string
    tft.fillRectangle(40, 90+90, 140, 110+90, COLOR_BLACK); //(x1,y1,x2,y2,color)
    
    tft.drawGFXText(x, y, storagestr, COLOR_WHITE); // Print string



    

  }

  distanceTemp = distance;
}

void handleRoot() {


  String html = "<html><head><meta http-equiv='refresh' content='1'>"
                "<style>body{background-color: #c3d9e0; font-size: 20px; margin: 50px; text-align: center;}"
                "h1{color: #333333; font-size: 36px; margin-bottom: 30px;}"
                "p{color: #333333; font-size: 24px;}"
                "span{font-style: italic;}"
                ".footer{display: flex; justify-content: flex-end; margin-top: 50px;}"
                "</style></head>";
  html += "<body><h1 style='background-color: #f8b195; padding: 20px; border-radius: 10px;'>Water Level</h1>"
          "<h2 style='margin-bottom: 20px;'>Fluid Measured: " + String(storage) + " cm</h2>"
          "<p>Refreshing every 1 second...</p>"
          "<div class='footer'><span>Fresia Medical Industry</span></div></body></html>";
          
          
  server.send(200, "text/html", html);
}




//
//void handleRoot() {
//  long duration, distance;
//  digitalWrite(trigPin, LOW);
//  delayMicroseconds(2);
//  digitalWrite(trigPin, HIGH);
//  delayMicroseconds(10);
//  digitalWrite(trigPin, LOW);
//  duration = pulseIn(echoPin, HIGH);
//  distance = duration * 0.034 / 2;
//
//  String html = "<html><head><meta http-equiv='refresh' content='1'>"
//                "<style>body{font-size: 20px; margin: 50px; text-align: center;} h1{font-size: 36px;} p{font-size: 24px;}</style></head>";
//  html += "<body><h1>Distance Measured: " + String(distance) + " cm</h1><p>Refreshing in 1 seconds...</p></body></html>";
//
//  server.send(200, "text/html", html);
//}
