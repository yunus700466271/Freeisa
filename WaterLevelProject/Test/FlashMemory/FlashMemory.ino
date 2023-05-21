#include <Keypad.h>
#include<Preferences.h>
#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {3, 21, 19, 18}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins 23, 3, 21, 19
byte pin_column[COLUMN_NUM] = {5, 17, 16, 4};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins 18, 5, 17, 16

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );

Preferences preferences;

// Function to store a value of any data type in the flash memory
template<typename T>
void storeValue(const char* variableName, T value) {
    preferences.begin("myApp", false);
   
    if (std::is_same<T, int>::value) {
        preferences.putInt(variableName, value);
    } else if (std::is_same<T, int16_t>::value) {
        preferences.putInt(variableName, value);
    } else if (std::is_same<T, long>::value) {
        preferences.putLong(variableName, value);
    } else if (std::is_same<T, float>::value) {
        preferences.putFloat(variableName, value);
    } else if (std::is_same<T, double>::value) {
        preferences.putDouble(variableName, value);
    } else if (std::is_same<T, bool>::value) {
        preferences.putBool(variableName, value);
    } else if (std::is_same<T, char>::value) {
        preferences.putString(variableName, String(value));
    } else {
        preferences.putString(variableName, String(value));
    }
   
    preferences.end();
}

// Function to get a value of any data type from the flash memory
template<typename T>
T getValue(const char* variableName, T defaultValue) {
    preferences.begin("myApp", true);
   
    T value;
   
    if (std::is_same<T, int>::value) {
        value = preferences.getInt(variableName, defaultValue);
    } else if (std::is_same<T, int16_t>::value) {
        value = preferences.getInt(variableName, defaultValue);
    } else if (std::is_same<T, long>::value) {
        value = preferences.getLong(variableName, defaultValue);
    } else if (std::is_same<T, float>::value) {
        value = preferences.getFloat(variableName, defaultValue);
    } else if (std::is_same<T, double>::value) {
        value = preferences.getDouble(variableName, defaultValue);
    } else if (std::is_same<T, bool>::value) {
        value = preferences.getBool(variableName, defaultValue);
    } else if (std::is_same<T, char>::value) {
        value = preferences.getString(variableName, String(defaultValue))[0];
    } else {
        value = preferences.getString(variableName, String(defaultValue)).toFloat();
    }
   
    preferences.end();
   
    return value;
}


// Function to delete a value with the given key from the flash memory
void deleteValue(const char* variableName) {
    preferences.begin("myApp", false);
    preferences.remove(variableName);
    preferences.end();  // Release the reserved storage for the app
}






// Function to delete all values for the app from the flash memory
void deleteAllValues() {
    preferences.begin("myApp", false);
    preferences.clear();   // Remove all preferences for the app
    preferences.end(); // Release the reserved storage for the app
}


void setup() {
  Serial.begin(115200);
}

void loop() {
  char key = keypad.getKey();

  if (key && key!='0') {
    Serial.println(key);
    storeValue("test", key);
  }
  if(key && key=='0') {
    deleteAllValues();
    char k = getValue("test", '0');
    Serial.println(k);
  }

  
}
