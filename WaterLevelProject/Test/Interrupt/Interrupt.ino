

uint32_t numberKeyPresses;
bool pressed;

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

byte row[ROW_NUM]      = {3, 21, 19, 18}; // GPIO19, GPIO18, GPIO5, GPIO17 connect to the row pins 
byte column[COLUMN_NUM] = {5, 17, 16, 4};   // GPIO16, GPIO4, GPIO0, GPIO2 connect to the column pins 

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

volatile char key;
//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0 , last_button_time = 0 , difference = 0;  
int button_row = -1;
int button_col = -1;

void IRAM_ATTR isr();

void setup() {
  Serial.begin(115200);

  for(int i = 0; i < COLUMN_NUM; i++){
    pinMode(column[i], OUTPUT);
    digitalWrite(column[i], LOW);
  }

  for(int i = 0; i < ROW_NUM; i++){
    pinMode(row[i], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(row[i]), isr, FALLING);
  }
}

void loop() {
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
