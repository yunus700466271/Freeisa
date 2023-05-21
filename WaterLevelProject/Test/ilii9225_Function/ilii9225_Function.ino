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
#define TFT_RS  27  // IO 25
#define TFT_CLK 14  // HSPI-SCK
// #define TFT_SDO 12  // HSPI-MISO
#define TFT_SDI 13  // HSPI-MOSI
#define TFT_CS  12  // HSPI-SS0
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










void setText(int16_t xr, int16_t yr ,String txt , bool box = true ,int16_t space = 20 , int16_t txtColor = COLOR_BLACK , int16_t borderSize = 3 , const GFXfont font = FreeSans9pt7b , int16_t boxColor = COLOR_GRAY ,int16_t borderColor = COLOR_BLACK) {


  int16_t wr,hr,h,w,x,y;

  if(box){
    tft.setGFXFont(&font); // Set current font
    tft.getGFXTextExtent(txt, x, y, &w, &h); // Get string extents
    wr = w*1.2+space - 10 ;
    hr = h + space  ; 

    tft.fillRectangle(xr , yr, xr+wr, yr+hr, borderColor); //(x1,y1,x2,y2,color)
    tft.fillRectangle(xr+borderSize , yr+borderSize, xr+wr-borderSize, yr+hr-borderSize, boxColor); //(x1,y1,x2,y2,color)
    
    x =xr + (wr-w)/2 - w*0.07;
    y = yr + (hr/2) + 2*borderSize ; // Set y position to string height
    tft.drawGFXText(x, y, txt, txtColor); // Print string
  }
  else{
    tft.setGFXFont(&font); // Set current font
    tft.getGFXTextExtent(txt, xr, yr, &w, &h); // Get string extents
    tft.drawGFXText(xr, yr, txt, txtColor); // Print string
    }

}






void setup() {

  Serial.begin(115200);

  // initialize the WiFi access point
  WiFi.softAP(ssid, password);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);




  
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


  tft.fillRectangle(0 , 0, 176, 220, COLOR_WHITE); //(x1,y1,x2,y2,color)
  setText(20, 80 ,"1-Set Text");
}


// Loop
void loop() {
  
  
}
