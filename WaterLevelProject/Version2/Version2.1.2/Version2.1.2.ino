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
#include<Preferences.h>







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
  {'.', '0', '#', 'D'}
};
byte row[ROW_NUM]      = {3, 21, 19, 18}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte column[COLUMN_NUM] = {5, 17, 16, 4};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Preferences preferences;
Keypad keypad = Keypad( makeKeymap(keys), row, column, ROW_NUM, COLUMN_NUM );
volatile char key = '0' , oldKey ;
volatile bool keyPressed = false;
int button_row = -1;
int button_col = -1;


TFT_22_ILI9225 tft = TFT_22_ILI9225(TFT_RST, TFT_RS, TFT_CS, TFT_LED, TFT_BRIGHTNESS);
int16_t pageNo = 0; //pageNo = 0 mainWindow

//Mode parameters
int16_t box_color = COLOR_GRAY , background_color = COLOR_WHITE , txt_color = COLOR_BLACK, border_color = COLOR_BLACK;



const char* ssid = "MyESP32AP"; // change to your desired access point name
const char* password = "password"; // change to your desired access point password
WebServer server(80);



long duration[4] = { 1000 , 2000, 3000 , 4000}, hight[4] = {100 , 200 , 300 , 400} , hightTemp[4]  , amount[4] ;
float tankHight[4] = {2000, 2000, 2000, 2000}, storage[4], tankGradiant[4];
String hightStr[4], amountstr[4], storagestr[4], tankHightStr[4], tankGradiantStr[4];
int counter = 0 , counterTemp = 0 , iTemp = 0;
String settingsPassword = "24434";



String getText(String);
int16_t getTextLength(String, const GFXfont);
int16_t setText(int16_t, int16_t, String, bool, int16_t, int16_t, int16_t, const GFXfont, int16_t, int16_t);
void window(int i);
void windowValues(int i);
void mainWindow();
void updateValues();
void IRAM_ATTR isr();  //Interrupt Function
void about();
void Password();
void settings();
void windowSelect();
void setParameters();
void setHights(int);
void setGradiants(int);
void viewParameters();
void advanced();
void setPassword();
void factoryReset();
void displayMode();


template<typename T>
void storeValue(char variableName[30], T value , int i);
template<typename T>
T getValue(char variableName[30], T defaultValue, int i);
void deleteValue(char* variableName, int i);
void deleteAllValues();
void init_();

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


  init_();
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


template<typename T>
void StoreValue(const char *variableName, String type , T value, int i) {

  char* varName = new char[strlen(variableName) + 10]; // allocate enough space for the final string
  strcpy(varName, variableName);
  if (i < 900) {
    char i_str[10];
    itoa(i, i_str, 10); // convert the integer i to a string
    strcat(varName, i_str);
  }

  Serial.print("Stored variable ");
  Serial.print(varName);
  Serial.print(" = ");
  Serial.println(value);

  preferences.begin("fluidLevel", false);

  if (type == "string") {
    preferences.putString(varName, String(value));
  }

  preferences.end();
  delete[] varName; // free the memory allocated for base_string

}

// Function to get a value of any data type from the flash memory
template<typename T>
T GetValue(const char *variableName, String type, T defaultValue , int i) {

  char* varName = new char[strlen(variableName) + 10]; // allocate enough space for the final string
  strcpy(varName, variableName);
  if (i < 900) {
    char i_str[10];
    itoa(i, i_str, 10); // convert the integer i to a string
    strcat(varName, i_str);
  }




  preferences.begin("fluidLevel", true);

  T value;


  if (type == "string") {
    value = preferences.getString(varName, String(defaultValue));
  }



  preferences.end();


  Serial.print("Get Stored variable ");
  Serial.print(varName);
  Serial.print(" = ");
  Serial.println(value);

  delete[] varName; // free the memory allocated for base_string
  return value;

}


// Function to store a value of any data type in the flash memory
template<typename T>
void storeValue(const char *variableName, T value, int i) {

  char* varName = new char[strlen(variableName) + 10]; // allocate enough space for the final string
  strcpy(varName, variableName);
  if (i < 900) {
    char i_str[10];
    itoa(i, i_str, 10); // convert the integer i to a string
    strcat(varName, i_str);
  }

  Serial.print("Stored variable ");
  Serial.print(varName);
  Serial.print(" = ");
  Serial.println(value);

  preferences.begin("fluidLevel", false);

  if (std::is_same<T, int>::value) {
    preferences.putInt(varName, value);
  } else if (std::is_same<T, int16_t>::value) {
    preferences.putInt(varName, value);
  } else if (std::is_same<T, long>::value) {
    preferences.putLong(varName, value);
  } else if (std::is_same<T, float>::value) {
    preferences.putFloat(varName, value);
  } else if (std::is_same<T, double>::value) {
    preferences.putDouble(varName, value);
  } else if (std::is_same<T, bool>::value) {
    preferences.putBool(varName, value);
  } else if (std::is_same<T, char>::value) {
    preferences.putString(varName, String(value));
  } else {
    preferences.putString(varName, String(value));
  }

  preferences.end();
  delete[] varName; // free the memory allocated for base_string
}

// Function to get a value of any data type from the flash memory
template<typename T>
T getValue(const char *variableName, T defaultValue , int i) {

  char* varName = new char[strlen(variableName) + 10]; // allocate enough space for the final string
  strcpy(varName, variableName);
  if (i < 900) {
    char i_str[10];
    itoa(i, i_str, 10); // convert the integer i to a string
    strcat(varName, i_str);
  }




  preferences.begin("fluidLevel", true);

  T value;

  if (std::is_same<T, int>::value) {
    value = preferences.getInt(varName, defaultValue);
  } else if (std::is_same<T, int16_t>::value) {
    value = preferences.getInt(varName, defaultValue);
  } else if (std::is_same<T, long>::value) {
    value = preferences.getLong(varName, defaultValue);
  } else if (std::is_same<T, float>::value) {
    value = preferences.getFloat(varName, defaultValue);
  } else if (std::is_same<T, double>::value) {
    value = preferences.getDouble(varName, defaultValue);
  } else if (std::is_same<T, bool>::value) {
    value = preferences.getBool(varName, defaultValue);
  } else if (std::is_same<T, char>::value) {
    value = preferences.getString(varName, String(defaultValue))[0];
  } else {
    value = preferences.getString(varName, String(defaultValue)).toFloat();
  }

  preferences.end();


  Serial.print("Get Stored variable ");
  Serial.print(varName);
  Serial.print(" = ");
  Serial.println(value);

  delete[] varName; // free the memory allocated for base_string
  return value;
}


// Function to delete a value with the given key from the flash memory
void deleteValue(const char* variableName, int i) {

  char* varName = new char[strlen(variableName) + 10]; // allocate enough space for the final string
  strcpy(varName, variableName);
  if (i < 900) {
    char i_str[10];
    itoa(i, i_str, 10); // convert the integer i to a string
    strcat(varName, i_str);
  }

  preferences.begin("fluidLevel", false);
  preferences.remove(varName);
  preferences.end();  // Release the reserved storage for the app
  Serial.print("Removed variable ");
  Serial.println(varName);
  delete[] varName; // free the memory allocated for base_string
}






// Function to delete all values for the app from the flash memory
void deleteAllValues() {
  preferences.begin("fluidLevel", false);
  preferences.clear();   // Remove all preferences for the app
  preferences.end(); // Release the reserved storage for the app
  Serial.print("Removed all variables ");
}







int16_t setText(int16_t xr, int16_t yr , String txt , bool box = true , int16_t horizontalSpace = 0, int16_t space = 20 , int16_t txtColor = txt_color , int16_t borderSize = 2 , const GFXfont font = FreeSans9pt7b , int16_t boxColor = box_color , int16_t borderColor = border_color) {


  int16_t wr, hr, h, w, x, y;

  if (box) {
    tft.setGFXFont(&font); // Set current font
    tft.getGFXTextExtent(txt, x, y, &w, &h); // Get string extents
    wr = w * 1.15 + space - 10  + horizontalSpace;
    hr = h + space  ;

    tft.fillRectangle(xr , yr, xr + wr, yr + hr, borderColor); //(x1,y1,x2,y2,color)
    tft.fillRectangle(xr + borderSize , yr + borderSize, xr + wr - borderSize, yr + hr - borderSize, boxColor); //(x1,y1,x2,y2,color)

    x = xr + (wr - w) / 2 - w * 0.05;
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








int16_t getTextLength(String s , const GFXfont font = FreeSans9pt7b) {
  int16_t x, y, w, h;
  tft.setGFXFont(&font); // Set current font
  tft.getGFXTextExtent(s, x, y, &w, &h); // Get string extents
  return w;
}


//template<typename Func>
String getText(String label ) { //,Func function) {

  String tempString;
  int w = getTextLength(label);

  keyPressed = false;
  key = 'N';
  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background


  setText((168 - w) / 2, 40, label , false); //but the string in the center
  setText(50, 180, "B-Back");
  setText(50, 140, "#-Save");
  while (key != '#' && key != 'B') {
    if (keyPressed) {


      int strLen = tempString.length() ;
      switch (key) {

        case 'A':
          if (strLen > 0) {
            tempString.remove(strLen - 1);
          }
          break;

        case 'B':
        case '#':
        case 'C':
        case 'D':
          break;

        default:
          tempString += key;
          break;


      }

      w = getTextLength(tempString);
      tft.fillRectangle(0 , 50, 176, 80, background_color); //set background

      setText((168 - w) / 2, 70, tempString, false);
      Serial.println(tempString);
      keyPressed = false;
    }
  }
  //function();
  if (key == 'B') {
    return "Back";
  }
  else {
    return tempString;
  }

}




void windowSelect() {



  if (keyPressed) {
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
        //        Password();
        settings();
        break;
      case 0:
        tft.clear();
        mainWindow();
        break;
    }
    keyPressed = false;
  }

  Serial.println("Exit windowSelect");


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
  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
  setText(10, 20, "1-Tank");
  setText(95, 20, "2-Tank");
  setText(10, 70, "3-Tank");
  setText(95, 70, "4-Tank");
  setText(30, 120, "5-Settings", true , 20);
  setText(30, 170, "6-About", true , 40);

}




void window(int i) {

  setText(20, 40 , "Fluid Level " + String(i) , false , 0, 20 , COLOR_LIGHTBLUE , 2 , FreeSans12pt7b);
  setText(58, 85 , "Height" , false, 0, 20 , COLOR_GREEN);
  setText(55, 130 , "Amount" , false, 0, 20 , COLOR_GREEN);
  setText(55, 175 , "Storage" , false, 0, 20 , COLOR_GREEN);


}



void windowValues(int i) {


  i -= 1;

  hightStr[i] = String(hight[i] / 10) + " Cm";
  amountstr[i] = String(amount[i]) + " Liters";
  storagestr[i] = String(storage[i]) + "%";



  if (int(hight[i]) != int(hightTemp[i]) || counter != counterTemp || i != iTemp) {


    counter++;



    tft.fillRectangle(40, 90, 140, 115, COLOR_BLACK); //(x1,y1,x2,y2,color)
    setText(50, 105 , hightStr[i] , false, 0, 20 , COLOR_WHITE);
    tft.fillRectangle(30, 135, 150, 155, COLOR_BLACK); //(x1,y1,x2,y2,color)
    setText(45, 150 , amountstr[i] , false, 0, 20 , COLOR_WHITE);
    tft.fillRectangle(40, 180, 140, 200, COLOR_BLACK); //(x1,y1,x2,y2,color)
    setText(50, 195 , storagestr[i] , false, 0, 20 , COLOR_WHITE);

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

  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background

  setText(20, 40, "This device was", false);
  setText(20, 60, "  Created by:", false);
  setText(25, 90, "Yunis Khamis", false);
  setText(15, 110, "+967773114988", false);
  setText(25, 140, "Asem Al-bajer", false);
  setText(15, 160, "+967773115313", false);
  setText(50, 180, "0-Home");

}


void Password() {

  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
  String tempStr = getText("Enter the password:");



  if (tempStr == settingsPassword) {
    settings();
  }
  else {
    if (tempStr == "Back") {
      mainWindow();
    }
    else {
      tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
      setText(10, 100, "Wrong Password!", false);
      delay(1000);
      mainWindow();
    }
  }


}

void settings() {

  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background



  setText(8, 20, "1-Set Parameters" , true , -10, 20 );
  setText(0, 70, "2-View Parameters" , true , -10, 20 );
  setText(22, 120, "3-Advanced", true , 10);

  setText(50, 180, "0-Home");

  key = 'N';

  while (key != '0') {
    if (keyPressed) {
      Serial.println(key);
      pageNo = key - 48;


      switch (pageNo) {

        case 1:
          setParameters();
          break;
        case 2:
          viewParameters();
          break;

        case 3:
          advanced();
          break;
        case 4:
          break;
      }
      keyPressed = false;
    }
  }
  Serial.println("Exit Settings");
  pageNo = 0;
  mainWindow();

}









void setParameters() {



  int x = 15, y = 10, xd = 90, yd = 40 ;
  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
  setText(x, y, "1-G1");
  setText(x, y + yd, "2-G2");
  setText(x, y + 2 * yd, "3-G3");
  setText(x, y + 3 * yd, "4-G4");
  setText(x + xd, y, "5-H1");
  setText(x + xd, y + yd, "6-H2");
  setText(x + xd, y + 2 * yd, "7-H3");
  setText(x + xd, y + 3 * yd, "8-H4");
  setText(50, 180, "0-Home");

  keyPressed = false;
  delay(500);


  while (key != '0') {
    if (keyPressed) {
      Serial.println(key);
      pageNo = key - 48;

      switch (pageNo) {

        case 1:
          setGradiants(pageNo - 1);
          break;
        case 2:
          setGradiants(pageNo - 1);
          break;
        case 3:
          setGradiants(pageNo - 1);
          break;
        case 4:
          setGradiants(pageNo - 1);
          break;
        case 5:
          setHights(pageNo - 5);
          break;
        case 6:
          setHights(pageNo - 5);
          break;
        case 7:
          setHights(pageNo - 5);
          break;
        case 8:
          setHights(pageNo - 5);
          break;
      }
      keyPressed = false;
    }
  }
  Serial.println("Exit Set Parameters");
  pageNo = 0;
  settings();
}







void setGradiants(int i) {


  String tempString = getText("Enter Gradiant " + String(i + 1) + ":");
  if (tempString.length() != 0 && tempString != "Back") {
    tankGradiantStr[i] = tempString;
    tankGradiant[i] = tankGradiantStr[i].toFloat();
    storeValue("tankGradiant_", tankGradiant[i], i);

    //For debug only
    char buf[15];
    dtostrf(tankGradiant[i], 15, 8, buf);
    Serial.println("Change parameter tankGradiant(" + String(i) + ") = " + String(buf));
    tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
    setText(15, 100, "Changes Saved", false);
    delay(1000);
  }
  Serial.println("Exit Set Gradiants");
  setParameters();
}



void setHights(int i) {


  String tempString = getText("Enter Height " + String(i + 1) + ":");
  if (tempString.length() != 0 && tempString != "Back") {
    tankHightStr[i] = tempString;
    tankHight[i] = tankHightStr[i].toFloat();
    storeValue("tankHight_", tankHight[i], i);

    //For debug only
    char buf[15];
    dtostrf(tankHight[i], 15, 8, buf);
    Serial.println("Change parameter tankHeight(" + String(i) + ") = " + String(buf));
    tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
    setText(15, 100, "Changes Saved", false);
    delay(1000);
  }
  Serial.println("Exit Set Heights");
  setParameters();


}




void viewParameters() {


  int x = 10, y = 20, xd = 40, yd = 20 ;
  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background


  setText(x, y, "1-G1:", false);
  setText(x, y + yd, "2-G2:", false);
  setText(x, y + 2 * yd, "3-G3:", false);
  setText(x, y + 3 * yd, "4-G4:", false);
  setText(x, y + 4 * yd, "5-H1:", false);
  setText(x, y + 5 * yd, "6-H2:", false);
  setText(x, y + 6 * yd, "7-H3:", false);
  setText(x, y + 7 * yd, "8-H4:", false);
  setText(50, 180, "0-Home");


  for (int i; i < 4; i++) {
    char tempChar[15];
    dtostrf(tankGradiant[i], 15, 8, tempChar);
    setText(x + xd, y + i * yd, String(tempChar) , false);
  }
  for (int i; i < 4; i++) {
    char tempChar[15];
    dtostrf(tankHight[i], 15, 8, tempChar);
    setText(x + xd, y + (i + 4)*yd, String(tempChar) , false);
  }


  while (key != '0');
  Serial.println("Exit View Parameters");
  settings();
}

void advancedw() {
  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
  setText(8, 20, "1-Set Password" );
  setText(8, 70, "2-Factory Reset" );
  setText(8, 120, "3-Display Mode" );

  setText(50, 180, "0-Home");


}

void advanced() {

  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background



  setText(8, 20, "1-Set Password" );
  setText(8, 70, "2-Factory Reset" );
  setText(8, 120, "3-Display Mode" );

  setText(50, 180, "0-Home");

  key = 'N';

  while (key != '0') {
    if (keyPressed) {
      Serial.println(key);
      pageNo = key - 48;


      switch (pageNo) {

        case 1:
          setPassword();
          break;
        case 2:
          factoryReset();
          break;

        case 3:
          displayMode();
          break;
        case 4:
          break;
      }
      keyPressed = false;
    }
  }
  //Serial.println("Enter Advanced");
  Serial.println("Exit Advanced");
  pageNo = 0;
  settings();
  delay(200);

}
void setPassword() {




  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
  String tempStr = getText("Enter the password:");
  String pass1, pass2;


  if (tempStr == settingsPassword) {
    delay(200);
    pass1 = getText("Enter New password:");
    delay(200);
    if (pass1 != "Back")
      pass2 = getText("Confirm password:");
    if (pass1 == pass2) {
      if (pass1.length() < 10 && pass1.length() > 3) {
        tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
        setText(0, 100, "password Changed!", false);
        delay(1000);
        settingsPassword = pass1;
        StoreValue("Password", "string", settingsPassword, 999);
        advancedw();
      }
      else {
        tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
        if (pass1.length() >= 10) {
          setText(0, 100, "Password tooLong!", false);
        }
        else if (pass1.length() <= 3) {
          setText(0, 100, "Password tooShort!", false);
        }

        delay(1000);
        advancedw();

      }
    }
    else {
      if (pass1 != "Back" && pass2 != "Back") {
        tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
        setText(10, 100, "passwords unmatch!", false);
        delay(1000);
      }
      advancedw();
    }
  }
  else {
    if (tempStr == "Back") {
      advancedw();
    }
    else {
      tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
      setText(10, 100, "Wrong Password!", false);
      delay(1000);
      advancedw();
    }
  }





  key = 'N';
  delay(200);


}
void factoryReset() {
  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
  String tempStr = getText("Enter Reset number:");

  int x1 = 30, y1 = 100, x2 = 45, y2 = 140;
  if (tempStr == "773114988") {
    tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
    setText(x1, y1, "Factory Reset", false);
    delay(1000);

    tft.fillRectangle(40 , 120, 150, 160, background_color); //set background
    setText(x1, y1, "Factory Reset", false);
    setText(x2, y2, "**", false);
    delay(1000);

    tft.fillRectangle(40 , 120, 150, 160, background_color); //set background
    setText(x1, y1, "Factory Reset", false);
    setText(x2, y2, "****", false);
    delay(1000);


    tft.fillRectangle(40 , 120, 150, 160, background_color); //set background
    setText(x1, y1, "Factory Reset", false);
    setText(x2, y2, "******", false);
    delay(1000);

    tft.fillRectangle(40 , 120, 150, 160, background_color); //set background
    setText(x1, y1, "Factory Reset", false);
    setText(x2, y2, "********", false);
    delay(1000);

    tft.fillRectangle(40 , 120, 150, 160, background_color); //set background
    setText(x1, y1, "Factory Reset", false);
    setText(x2, y2, "**********", false);
    delay(1000);

    deleteAllValues();
    init_();
    advancedw();
  }
  else {
    if (tempStr == "Back") {
      advancedw();
    }
    else {
      tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
      setText(10, 100, "Wrong Password!", false);
      delay(1000);
      advancedw();
    }
  }
  key = 'N';
  delay(200);

}

void displayMode() {

  int x = 10, y = 20, yd = 40;
  tft.fillRectangle(0 , 0, 176, 220, background_color); //set background
  setText(x, y, "1-Light Mode", true, 22);
  setText(x, y + yd, "2-Dark Mode", true, 22);
  setText(x, y + 2 * yd, "3-Colorfull Mode", true, -10);
  setText(x, y + 3 * yd, "4-Relieve Mode", true, -3);

  setText(50, 180, "0-Home");
  key = 'N';
  while (key != '0') {
    if (keyPressed) {
      Serial.println(key);
      pageNo = key - 48;


      switch (pageNo) {

        case 1:
          box_color = COLOR_GRAY;
          background_color = COLOR_WHITE;
          txt_color = COLOR_BLACK;
          border_color = COLOR_BLACK;
          break;
        case 2:
          box_color = COLOR_GRAY;
          background_color = COLOR_BLACK; //0x4A148C-GRASSGREEN
          txt_color = COLOR_WHITE;
          border_color = 0xFFFFCC; //0xFFFFCC-DARKYALLOW
          break;

        case 3:
          box_color = 0xFFE5B4;//0xFFE5B4-LIGHTPINK
          background_color = 0xE6E6FA;//0xE6E6FA-LAVENDER
          txt_color = COLOR_BLACK;
          border_color = COLOR_BLACK;
          break;
        case 4:
          box_color = 0x89CFF0; //    0xFFCCE5-LIGHTYALLOW     0xFFE5B4-LIGHTPINK   0x98FB98-DARKPINK 0xFFFFCC-DARKYALLOW 0xFADADD-PURPLE
          background_color = 38193; //38193-SKYBLUE     0x87CEEB-LIGHTGREEN  0xE6E6FA-LAVENDER 0x89CFF0-BABYGREEN
          txt_color = COLOR_BLACK;
          border_color = COLOR_BLACK;
          break;
      }
      keyPressed = false;
      if (key == '1' || key == '2' || key == '3' || key == '4' ) {
        break;
      }
    }
  }
  storeValue("box_color", box_color, 999);
  storeValue("background", background_color, 999);
  storeValue("txt_color", txt_color, 999);
  storeValue("border_color", border_color, 999);

  Serial.println("Exit Display Mode");
  pageNo = 0;
  key = 'N';
  advancedw();
  delay(200);

}




void init_() {


  for (int i; i < 4; i++) {
    tankGradiant[i] = getValue("tankGradiant_", float(2.55), i);
  }
  for (int i; i < 4; i++) {
    tankHight[i] = getValue("tankHight_", float(2000), i);
  }

  box_color = getValue("box_color", COLOR_GRAY, 999);
  background_color = getValue("background", COLOR_WHITE, 999);
  txt_color = getValue("txt_color", COLOR_BLACK, 999);
  border_color = getValue("border_color", COLOR_BLACK, 999);
  settingsPassword = GetValue("Password", "string", settingsPassword, 999);
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
