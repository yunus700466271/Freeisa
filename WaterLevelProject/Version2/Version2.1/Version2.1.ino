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
const int echoPin[4] = {36 ,39 ,34 ,35 }; // change to the actual pin number that the JSN-SR04T echo pin is connected to



char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte row[ROW_NUM]      = {3, 21, 19, 18}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins 
byte column[COLUMN_NUM] = {5, 17, 16, 4};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins 


Keypad keypad = Keypad( makeKeymap(keys), row, column, ROW_NUM, COLUMN_NUM );
volatile char key = '0' , oldKey;

int button_row = -1;
int button_col = -1;


TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
int16_t x=0, y=20, w, h , pageNo = 0; //pageNo = 0 mainWindow
int16_t xr=0, yr=20, wr, hr , tr , s , t = 20;

const char* ssid = "MyESP32AP"; // change to your desired access point name
const char* password = "password"; // change to your desired access point password
WebServer server(80);



long duration[4] = { 1000 , 2000, 3000 , 4000}, hight[4] = {100 , 200 , 300 , 400} , hightTemp[4]  , amount[4] ;
float tankHight[4] = {2000,2000,2000,2000},storage[4];
String hightStr[4],amountstr[4],storagestr[4];
int counter = 0 , counterTemp = 0 , iTemp = 0;




void window(int i);
void windowValues(int i);
void mainWindow();
void updateValues();
void IRAM_ATTR isr();  //Interrupt Function
void about();
void settings();
void windowSelect();

void setup() {
  Serial.begin(115200);






  //Interrupt Setup
  for(int i = 0; i < COLUMN_NUM; i++){
    pinMode(column[i], OUTPUT);
    digitalWrite(column[i], LOW);
  }

  for(int i = 0; i < ROW_NUM; i++){
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






  


  



  for(int i = 0; i<4 ; i++){
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
  
  if(pageNo>0 && pageNo<5 && currentTime - lastTime >= 1000){     //Print the updated values every 1 second
      Serial.println("Print Updated values to Display");
      windowValues(pageNo);
      lastTime = currentTime;
    }
}




void windowSelect(){
  
  
    
  if (key != oldKey) {
    Serial.println(key);
    pageNo = key - 48;  


    switch(pageNo){
      
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

void updateValues(){
  
  for(int i = 0;i < 4 ; i++){
    
      digitalWrite(trigPin[i], LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin[i], HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin[i], LOW);
      duration[i] = pulseIn(echoPin[i], HIGH);
      hight[i] = tankHight[i]-(duration[i] * 0.34 / 2);
      amount[i] = 2.519011824 * hight[i];
      storage[i] = 100*(hight[i]/tankHight[i]);
    }
  }










void mainWindow(){
  
  x=0;
  y=15;

  counter++;
  tft.fillRectangle(0 , 0, 176, 220, COLOR_WHITE); //set background


  
  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent("1-Tank", x, y, &w, &h); // Get string extents
  
  //Set the coordinations of the box
  xr = 15 ; yr = 15 ; wr = 65 ; hr = 30 ; tr = 2 ;s = 15; 
  tft.fillRectangle(xr , yr, xr+wr, yr+hr, COLOR_BLACK); //(x1,y1,x2,y2,color)
  tft.fillRectangle(xr+tr , yr+tr, xr+wr-tr, yr+hr-tr, COLOR_GRAY); //(x1,y1,x2,y2,color)

  y = y + yr + 5 ; // Set y position to string height
  x = x + xr + 2;
  tft.drawGFXText(x, y, "1-Tank", COLOR_BLACK); // Print string




  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent("2-Tank", x, y, &w, &h); // Get string extents
  
  //Set the coordinations of the box
  xr = xr + wr +s  ; yr = yr ;
  tft.fillRectangle(xr , yr, xr+wr, yr+hr, COLOR_BLACK); //(x1,y1,x2,y2,color)
  tft.fillRectangle(xr+tr , yr+tr, xr+wr-tr, yr+hr-tr, COLOR_GRAY); //(x1,y1,x2,y2,color)
  x = x + wr + s;
  y = y;
  tft.drawGFXText(x, y, "2-Tank", COLOR_BLACK); // Print string








  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent("3-Tank", x, y, &w, &h); // Get string extents
  
  //Set the coordinations of the box
  xr = xr - (wr +s)  ; yr = yr + hr + s ;
  tft.fillRectangle(xr , yr, xr+wr, yr+hr, COLOR_BLACK); //(x1,y1,x2,y2,color)
  tft.fillRectangle(xr+tr , yr+tr, xr+wr-tr, yr+hr-tr, COLOR_GRAY); //(x1,y1,x2,y2,color)
  x = x - (wr + s);
  y = y + (hr + s);
  tft.drawGFXText(x, y, "3-Tank", COLOR_BLACK); // Print string









  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent("4-Tank", x, y, &w, &h); // Get string extents
  
  //Set the coordinations of the box
  xr = xr + (wr +s)  ; yr = yr  ;
  tft.fillRectangle(xr , yr, xr+wr, yr+hr, COLOR_BLACK); //(x1,y1,x2,y2,color)
  tft.fillRectangle(xr+tr , yr+tr, xr+wr-tr, yr+hr-tr, COLOR_GRAY); //(x1,y1,x2,y2,color)
  x = x + (wr + s);
  y = y ;
  tft.drawGFXText(x, y, "4-Tank", COLOR_BLACK); // Print string



  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent("5-Settings", x, y, &w, &h); // Get string extents

  
  //Set the coordinations of the box
  xr = 23 ; yr = yr + hr + s ; wr = 130 ; hr = 30 ; tr = 2 ;
  tft.fillRectangle(xr , yr, xr+wr, yr+hr, COLOR_BLACK); //(x1,y1,x2,y2,color)
  tft.fillRectangle(xr+tr , yr+tr, xr+wr-tr, yr+hr-tr, COLOR_GRAY); //(x1,y1,x2,y2,color)
  x = 0;
  x = xr + (wr-w)/2 -5;
  y = y + (hr + s);
  tft.drawGFXText(x, y, "5-Settings", COLOR_BLACK); // Print string






  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent("6-About", x, y, &w, &h); // Get string extents

  
  //Set the coordinations of the box
  xr = xr ; yr = yr + hr + s ; 
  tft.fillRectangle(xr , yr, xr+wr, yr+hr, COLOR_BLACK); //(x1,y1,x2,y2,color)
  tft.fillRectangle(xr+tr , yr+tr, xr+wr-tr, yr+hr-tr, COLOR_GRAY); //(x1,y1,x2,y2,color)
  x = xr + (wr-w)/2 ;
  y = y + (hr + s);
  tft.drawGFXText(x, y, "6-About", COLOR_BLACK); // Print string




  }




void window(int i){

  
  x=0; y=20;
  

  String title = "Fluid Level " + String(i);

  tft.setGFXFont(&FreeSans12pt7b); // Set current font
  tft.getGFXTextExtent(title, x, y, &w, &h); // Get string extents
  y += h; // Set y position to string height
  x += 20;
  tft.drawGFXText(x, y,title , COLOR_CYAN); // Print string


  
  tft.setGFXFont(&FreeSans9pt7b);  // Change font
  tft.getGFXTextExtent(" Hight", x, y, &w, &h); // Get string extents
  y += h + 33; // Set y position to string height plus shift down 10 pixels
  x+=35;
  tft.drawGFXText(x, y, " Height", COLOR_GREEN); // Print string
  
  
  
  tft.setGFXFont(&FreeSans9pt7b);  // Change font
  tft.getGFXTextExtent(" Amount", x, y, &w, &h); // Get string extents
  y += h + 33; // Set y position to string height plus shift down 10 pixels
  x-=5;
  tft.drawGFXText(x, y, " Amount", COLOR_GREEN); // Print string
  
  
  
  tft.setGFXFont(&FreeSans9pt7b);  // Change font
  tft.getGFXTextExtent(" Storage", x, y, &w, &h); // Get string extents
  y += h + 33; // Set y position to string height plus shift down 10 pixels
  tft.drawGFXText(x, y, " Storage", COLOR_GREEN); // Print string
  
  }



void windowValues(int i){
  
  
  x=0;y=60;i -=1;

  hightStr[i] = String(hight[i]/10) + " Cm";
  amountstr[i] =String(amount[i]) + " Liters";
  storagestr[i] = String(storage[i]) + "%";
  
  //server.handleClient();

  
  if(int(hight[i])!=int(hightTemp[i]) || counter != counterTemp || i!= iTemp){

    
    counter++;
    
    tft.setGFXFont(&FreeSans9pt7b);  // Change font
    tft.getGFXTextExtent(hightStr[i], x, y, &w, &h); // Get string extents
    y += h + 33; // Set y position to string height plus shift down 10 pixels
    x+= 55;
    // Draw solid rectangular to clear the string
    tft.fillRectangle(40, 90, 140, 110, COLOR_BLACK); //(x1,y1,x2,y2,color)
    
    tft.drawGFXText(x, y, hightStr[i], COLOR_WHITE); // Print string




    tft.setGFXFont(&FreeSans9pt7b);  // Change font
    tft.getGFXTextExtent(amountstr[i], x, y, &w, &h); // Get string extents
    y += h + 33; // Set y position to string height plus shift down 10 pixels
    x-= 10;
    // Draw solid rectangular to clear the string
    tft.fillRectangle(40, 90+45, 140, 110+45, COLOR_BLACK); //(x1,y1,x2,y2,color)
    
    tft.drawGFXText(x, y, amountstr[i], COLOR_WHITE); // Print string


    tft.setGFXFont(&FreeSans9pt7b);  // Change font
    tft.getGFXTextExtent(storagestr[i], x, y, &w, &h); // Get string extents
    y += h + 33; // Set y position to string height plus shift down 10 pixels
    x+=12;
    // Draw solid rectangular to clear the string
    tft.fillRectangle(40, 90+90, 140, 110+90, COLOR_BLACK); //(x1,y1,x2,y2,color)
    
    tft.drawGFXText(x, y, storagestr[i], COLOR_WHITE); // Print string



    

  }

  hightTemp[i] = hight[i];
  counterTemp = counter;
  iTemp = i;
  
  }







void IRAM_ATTR isr() {
//  

    

    
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
    
    Serial.printf("Button at row %d and column %d character (%c) \n", button_row, button_col, key);
    }

    if(button_row ==0 && button_col == 0){
      key = keys[button_row][button_col];
      Serial.printf("Button at row %d and column %d character (%c) \n", button_row, button_col, key);
      }
    

    
}



void about(){
  
  
  
    
  x=10;
  y=70;
  
  tft.fillRectangle(0 , 0, 176, 220, COLOR_WHITE); //set background



  String s = " This device was";
  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent(s, x, y, &w, &h); // Get string extents

  tft.drawGFXText(x, y,s, COLOR_BLACK); // Print string
  
  x = 0;
  
  s = "  programmed by:";
  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent(s, x, y, &w, &h); // Get string extents
  y+=20;
  tft.drawGFXText(x, y, s, COLOR_BLACK); // Print string



  

  s = "   Yunis Khamis";
  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent(s, x, y, &w, &h); // Get string extents
  y+=40;
  x+=12;
  tft.drawGFXText(x, y, s, COLOR_BLACK); // Print string



  s = "   +967773114988";
  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent(s, x, y, &w, &h); // Get string extents
  y+=25;
  x-=15;
  tft.drawGFXText(x, y, s, COLOR_BLACK); // Print string




  
  
  
  }


void settings(){
  
  
  x=0;
  y=15;

  counter++;
  tft.fillRectangle(0 , 0, 176, 220, COLOR_WHITE); //set background


  
  tft.setGFXFont(&FreeSans9pt7b); // Set current font
  tft.getGFXTextExtent("Set parameters", x, y, &w, &h); // Get string extents
  x = x + 20;
  y = y + 10; // Set y position to string height
  tft.drawGFXText(x, y, "Set parameters", COLOR_BLACK); // Print string
  


  t = 20;
  tft.getGFXTextExtent("hamod", x, y, &w, &h); // Get string extents

  wr = w+t ; hr = h + t - 5 ; tr = 2 ;
  s = (176 - (wr))/2;
  xr = 100 ; yr = 150 ;  
  tft.fillRectangle(xr , yr, xr+wr, yr+hr, COLOR_BLACK); //(x1,y1,x2,y2,color)
  tft.fillRectangle(xr+tr , yr+tr, xr+wr-tr, yr+hr-tr, COLOR_GRAY); //(x1,y1,x2,y2,color)
  
  x =xr + (wr-w)/2;
  y = yr + (hr-h) + 5; // Set y position to string height
  tft.drawGFXText(x, y, "hamod", COLOR_BLACK); // Print string
  

  
  
  
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
