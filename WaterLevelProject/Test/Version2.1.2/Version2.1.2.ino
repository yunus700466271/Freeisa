#include <Keypad.h>
#include "SPI.h"
#include "TFT_22_ILI9225.h"
#include <../fonts/FreeSans18pt7b.h>
#include <../fonts/FreeSans12pt7b.h>
#include <../fonts/FreeSans9pt7b.h>

#include <WiFi.h>
#include <WebServer.h>
#include <Arduino.h>
#include "esp32-hal-psram.h"








#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns
#define TFT_RST 26  // IO 26
#define TFT_RS  27  // IO 25
#define TFT_CLK 14  // HSPI-SCK
// #define TFT_SDO 12  // HSPI-MISO
#define TFT_SDI 13  // HSPI-MOSI
#define TFT_CS  12  // HSPI-SS0
#define TFT_LED 0   // 0 if wired to +5V directly
SPIClass hspi(HSPI);
#define TFT_BRIGHTNESS 200 // Initial brightness of TFT backlight (optional)









const int trigPin[4] = {23 , 22 , 2 , 15}; // change to the actual pin number that the JSN-SR04T trigger pin is connected to
const int echoPin[4] = {36 , 39 , 34 , 35 }; // change to the actual pin number that the JSN-SR04T echo pin is connected to



char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte row[ROW_NUM]      = {3, 21, 19, 18}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte column[COLUMN_NUM] = {5, 17, 16, 4};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins


Keypad keypad = Keypad( makeKeymap(keys), row, column, ROW_NUM, COLUMN_NUM );
volatile char key = '0' , oldKey ;
volatile bool keyPressed = false;
int button_row = -1;
int button_col = -1;


TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
int16_t x = 0, y = 20, w, h , pageNo = 0; //pageNo = 0 mainWindow
int16_t xr = 0, yr = 20, wr, hr , tr , s , t = 20;

const char* ssid = "MyESP32AP"; // change to your desired access point name
const char* password = "password"; // change to your desired access point password
WebServer server(80);



long duration[4] = { 1000 , 2000, 3000 , 4000}, hight[4] = {100 , 200 , 300 , 400} , hightTemp[4]  , amount[4] ;
float tankHight[4] = {2000, 2000, 2000, 2000}, storage[4], tankConstant[4];
String hightStr[4], amountstr[4], storagestr[4], tankHightStr[4], tankConstantStr[4];
int counter = 0 , counterTemp = 0 , iTemp = 0;




class Text{
  
  public:
  int16_t xr;
  int16_t yr;
  String txt;
  bool box = true;
  int16_t horizontalSpace = 0;
  int16_t space = 20;
  int16_t txtColor = COLOR_BLACK;
  int16_t borderSize = 2;
  GFXfont font = FreeSans9pt7b;
  int16_t boxColor = COLOR_GRAY;
  int16_t borderColor = COLOR_BLACK;
  
  Text(int16_t xr_, int16_t yr_ , String txt_ , bool box_ = true , int16_t horizontalSpace_ = 0, int16_t space_ = 20 , int16_t txtColor_ = COLOR_BLACK , int16_t borderSize_ = 2 ,GFXfont font_ = FreeSans9pt7b , int16_t boxColor_ = COLOR_GRAY , int16_t borderColor_ = COLOR_BLACK);
  };
String getText(String label);
int16_t setText(int16_t, int16_t, String, bool, int16_t, int16_t, int16_t, const GFXfont, int16_t, int16_t);
void window(int i);
void windowValues(int i);
void mainWindow();
void updateValues();
void IRAM_ATTR isr();  //Interrupt Function
void about();
void settings();
void windowSelect();
void tankSettings(int);

void setup() {
  Serial.begin(115200);






  //Interrupt Setup
  for (int i = 0; i < COLUMN_NUM; i++) {
    pinMode(column[i], OUTPUT);
    digitalWrite(column[i], LOW);
  }

  for (int i = 0; i < ROW_NUM; i++) {
    pinMode(row[i], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(row[i]), isr, FALLING);
  }







  //Print the usage of resources

  delay(200);
  size_t sketch_size = ESP.getSketchSize();
  size_t free_space = ESP.getFreeSketchSpace();
  size_t heap_size = ESP.getHeapSize();
  size_t free_heap = ESP.getFreeHeap();

  Serial.print("Sketch size: ");
  Serial.print(sketch_size / 1024);
  Serial.println(" KB");

  Serial.print("Free space: ");
  Serial.print(free_space / 1024);
  Serial.println(" KB");

  float flash_usage_percentage = 100.0 * (float)(sketch_size) / (float)(sketch_size + free_space);

  Serial.print("Flash memory usage: ");
  Serial.print(flash_usage_percentage);
  Serial.println("%");

  Serial.print("Heap size: ");
  Serial.print(heap_size / 1024);
  Serial.println(" KB");

  Serial.print("Free heap: ");
  Serial.print(free_heap / 1024);
  Serial.println(" KB");

  float ram_usage_percentage = 100.0 * (float)(heap_size - free_heap) / (float)(heap_size);

  Serial.print("RAM memory usage: ");
  Serial.print(ram_usage_percentage);
  Serial.println("%");













  for (int i = 0; i < 4 ; i++) {
    pinMode(trigPin[i], OUTPUT);
    pinMode(echoPin[i], INPUT);
  }












  // initialize the WiFi access point
  WiFi.softAP(ssid, password);
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



  mainWindow();

}







void loop() {


  static unsigned long lastTime = 0;
  unsigned long currentTime = millis();

  //server.handleClient();

  windowSelect();

  updateValues();

  if (pageNo > 0 && pageNo < 5 && currentTime - lastTime >= 1000) { //Print the updated values every 1 second
    Serial.println("Print Updated values to Display");
    windowValues(pageNo);
    lastTime = currentTime;
  }

  if (currentTime - lastTime >= 1000) {   //Print the updated values every 1 second
    Serial.println(key);
    lastTime = currentTime;
  }
}











int16_t setText(int16_t xr, int16_t yr , String txt , bool box = true , int16_t horizontalSpace = 0, int16_t space = 20 , int16_t txtColor = COLOR_BLACK , int16_t borderSize = 2 , const GFXfont font = FreeSans9pt7b , int16_t boxColor = COLOR_GRAY , int16_t borderColor = COLOR_BLACK) {


  int16_t wr, hr, h, w, x, y;

  if (box) {
    tft.setGFXFont(&font); // Set current font
    tft.getGFXTextExtent(txt, x, y, &w, &h); // Get string extents
    wr = w * 1.15 + space - 10  + horizontalSpace;
    hr = h + space  ;

    tft.fillRectangle(xr , yr, xr + wr, yr + hr, borderColor); //(x1,y1,x2,y2,color)
    tft.fillRectangle(xr + borderSize , yr + borderSize, xr + wr - borderSize, yr + hr - borderSize, boxColor); //(x1,y1,x2,y2,color)

    x = xr + (wr - w) / 2 - w * 0.07;
    y = yr + (hr / 2) + 2 * borderSize ; // Set y position to string height
    tft.drawGFXText(x, y, txt, txtColor); // Print string
  }
  else {
    tft.setGFXFont(&font); // Set current font
    tft.getGFXTextExtent(txt, xr, yr, &w, &h); // Get string extents
    tft.drawGFXText(xr, yr, txt, txtColor); // Print string
  }
  return w;

}















void windowSelect() {



  if (key != oldKey) {
    Serial.println(key);
    pageNo = key - 48;


    switch (pageNo) {

      case 1:
      case 2:
      case 3:
      case 4:
        tft.clear();
        window(pageNo);
        break;

      case 6:
        tft.clear();
        about();
        break;
      case 5:
        tft.clear();
        settings();
        break;
      case 0:
        tft.clear();
        mainWindow();
        break;
    }
    oldKey = key;
  }



}

void updateValues() {

  for (int i = 0; i < 4 ; i++) {

    digitalWrite(trigPin[i], LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin[i], HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin[i], LOW);
    duration[i] = pulseIn(echoPin[i], HIGH);
    hight[i] = tankHight[i] - (duration[i] * 0.34 / 2);
    amount[i] = 2.519011824 * hight[i];
    storage[i] = 100 * (hight[i] / tankHight[i]);
  }
}










void mainWindow() {
  counter++;
  tft.fillRectangle(0 , 0, 176, 220, COLOR_WHITE); //set background
  Text(10, 20, "1-Tank");
  Text(95, 20, "2-Tank");
  Text(10, 70, "3-Tank");
  Text(95, 70, "4-Tank");
  Text(30, 120, "5-Settings", true , 20);
  Text(30, 170, "6-About", true , 40);

}




void window(int i) {

  Text(20, 40 , "Fluid Level " + String(i) , false , 0, 20 , COLOR_LIGHTBLUE , 2 , FreeSans12pt7b);
  Text(58, 85 , "Height" , false, 0, 20 , COLOR_GREEN);
  Text(55, 130 , "Amount" , false, 0, 20 , COLOR_GREEN);
  Text(55, 175 , "Storage" , false, 0, 20 , COLOR_GREEN);


}



void windowValues(int i) {


  i -= 1;

  hightStr[i] = String(hight[i] / 10) + " Cm";
  amountstr[i] = String(amount[i]) + " Liters";
  storagestr[i] = String(storage[i]) + "%";



  if (int(hight[i]) != int(hightTemp[i]) || counter != counterTemp || i != iTemp) {


    counter++;



    tft.fillRectangle(40, 90, 140, 115, COLOR_BLACK); //(x1,y1,x2,y2,color)
    Text(50, 105 , hightStr[i] , false, 0, 20 , COLOR_WHITE);
    tft.fillRectangle(30, 135, 150, 155, COLOR_BLACK); //(x1,y1,x2,y2,color)
    Text(45, 150 , amountstr[i] , false, 0, 20 , COLOR_WHITE);
    tft.fillRectangle(40, 180, 140, 200, COLOR_BLACK); //(x1,y1,x2,y2,color)
    Text(50, 195 , storagestr[i] , false, 0, 20 , COLOR_WHITE);

  }

  hightTemp[i] = hight[i];
  counterTemp = counter;
  iTemp = i;

}







void IRAM_ATTR isr() {

  static unsigned long count = 0;


  for (int i = 0; i < ROW_NUM; i++) {

    if (digitalRead(row[i]) == LOW) {
      button_row = i;
      break;
    }

  }
  for (int i = 0; i < COLUMN_NUM; i++) {

    digitalWrite(column[i], HIGH);
    if (digitalRead(row[button_row]) == HIGH) {
      button_col = i;
      digitalWrite(column[i], LOW);
      break;
    }
    digitalWrite(column[i], LOW);
    key = keys[button_row][button_col];
    //Serial.printf("Button at row %d and column %d character (%c) \n", button_row, button_col, key);
  }
  //    if (key != 'A' && key != '3') {
  if (button_row == 0 && button_col == 0) {
    key = keys[button_row][button_col];
    //Serial.printf("Button at row %d and column %d character (%c) \n", button_row, button_col, key);
    //      }
  }


  count++;

  if (count == 30000) { //Print the updated values every 1 second
    Serial.printf("============== (%c) ============== \n", key);
    count = 0;
    keyPressed = true;
  }

}



void about() {

  tft.fillRectangle(0 , 0, 176, 220, COLOR_WHITE); //set background

  Text(20, 40, "This device was", false);
  Text(20, 60, "  Created by:", false);
  Text(25, 90, "Yunis Khamis", false);
  Text(15, 110, "+967773114988", false);
  Text(25, 140, "Asem Al-bajer", false);
  Text(15, 160, "+967773115313", false);
  Text(50, 180, "0-Home");

}

void settings() {

  tft.fillRectangle(0 , 0, 176, 220, COLOR_WHITE); //set background

  Text(20, 20, "Set Parameters", false);
  Text(10, 60, "1-Tank");
  Text(95, 60, "2-Tank");
  Text(10, 100, "3-Tank");
  Text(95, 100, "4-Tank");
  Text(50, 180, "0-Home");

  while (key != '0') {
    if (key != oldKey) {

      Serial.println(key);
      pageNo = key - 48;


      switch (pageNo) {

        case 1:
          tankSettings(1);
          break;
        case 2:
          tankSettings(2);
          break;
        case 3:
          tankSettings(3);
          break;
        case 4:
          tankSettings(4);
          break;
      }
      oldKey = key;
    }
  }
  Serial.println("Exit Settings");
  mainWindow();
}

void tankSettings(int i) {
  keyPressed = false;
  tft.fillRectangle(0 , 0, 176, 220, COLOR_WHITE); //set background
  oldKey = key;
  String test = getText("aaaaadddddfffff");

  settings();
}








String getText(String label) {



  String temp;
  Text(0, 40, label ,false);
  Text(50, 180, "B-Back");
  Text(50, 140, "#-Save");
  while (key != '#') {
    if (keyPressed) {

      if (key != 'A') {
        temp += key;
      }
      else {
        int strLen = temp.length() ;
        if (strLen > 0) {
          temp.remove(strLen - 1);
        }
      }
      tft.fillRectangle(0 , 50, 176, 80, COLOR_RED); //set background
      Text(20, 70, temp, false);
      Serial.println(temp);
      oldKey = key;
      keyPressed = false;
    }
  }

  return temp;



}




















Text::Text(int16_t xr_, int16_t yr_ , String txt_ , bool box_ ,int16_t horizontalSpace_, int16_t space_ , int16_t txtColor_, int16_t borderSize_,GFXfont font_ , int16_t boxColor_ , int16_t borderColor_) {
  
  xr = xr_;
  yr = yr_;
  txt = txt_;
  box = box_;
  horizontalSpace = horizontalSpace_;
  space = space_;
  txtColor = txtColor_;
  borderSize = borderSize_;
  font = font_;
  boxColor = boxColor_;
  borderColor = borderColor_;


  int16_t wr, hr, h, w, x, y;

  if (box) {
    tft.setGFXFont(&font); // Set current font
    tft.getGFXTextExtent(txt, x, y, &w, &h); // Get string extents
    wr = w * 1.15 + space - 10  + horizontalSpace;
    hr = h + space  ;

    tft.fillRectangle(xr , yr, xr + wr, yr + hr, borderColor); //(x1,y1,x2,y2,color)
    tft.fillRectangle(xr + borderSize , yr + borderSize, xr + wr - borderSize, yr + hr - borderSize, boxColor); //(x1,y1,x2,y2,color)

    x = xr + (wr - w) / 2 - w * 0.07;
    y = yr + (hr / 2) + 2 * borderSize ; // Set y position to string height
    tft.drawGFXText(x, y, txt, txtColor); // Print string
  }
  else {
    tft.setGFXFont(&font); // Set current font
    tft.getGFXTextExtent(txt, xr, yr, &w, &h); // Get string extents
    tft.drawGFXText(xr, yr, txt, txtColor); // Print string
  }


  
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
          "<h2 style='margin-bottom: 20px;'>Fluid Measured: " + String(55) + " cm</h2>"
          "<p>Refreshing every 1 second...</p>"
          "<div class='footer'><span>Fresia Medical Industry</span></div></body></html>";


  server.send(200, "text/html", html);
}
