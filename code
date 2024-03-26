#include <EEPROM.h>

// needed for display
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

int ledState1 = 2;
int ledState2 = 3;
int relaisPin = 12;
int changeStateBtn = 10;
int startMillBtn = 9;
int lowerTimeBtn = A1;
int raiseTimeBtn = A2;

int stateBtn = 0;
bool isMilling = false;

// https://www.mischianti.org/2021/07/14/ssd1306-oled-display-draw-images-splash-and-animations-2/#1_Select_image
// 'cup', 16x16px
const unsigned char cup [] PROGMEM = {
  0x0d, 0xa0, 0x09, 0x60, 0x05, 0xb0, 0x06, 0x90, 0x04, 0x20, 0x00, 0x00, 0x1f, 0xfe, 0x3f, 0xff, 
  0x20, 0x0d, 0x20, 0x0f, 0x20, 0x0e, 0x30, 0x08, 0x10, 0x08, 0x08, 0x10, 0x07, 0xe0, 0xff, 0xff
};


int timeState1 = 0;
int timeState2 = 0;

void setup() {
  setupPins();
  Serial.begin(9600);

  // initialize state, this will be used for timing later
  stateBtn = 2;
  
  // read timer values from EEPROM
  timeState1 = (readIntFromEEPROM(0) * 20);
  timeState2 = (readIntFromEEPROM(2) * 20);

  // if there's a problem with the EEPROM, fall back to somewhat plausible values
  if(timeState1 < 0) { timeState1 = 3600; }
  if(timeState2 < 0) { timeState2 = 7200; }

  // update LEDs + display to reflect current state
  updateState();

  Serial.println("######################################");
  Serial.println("Machine started.");
  Serial.println("Available modes:");
  Serial.println("0 - manual");
  Serial.println("1 - single shot (timeState1)");
  Serial.println("2 - double shot (timeState2)");
  Serial.print("Machine mode is now (by default): ");
  Serial.println((int) stateBtn);
  Serial.println("Saved timeState values are:");
  Serial.print("timeState1: ");
  Serial.println(timeState1);
  Serial.print("timeState2: ");
  Serial.println(timeState2);
  Serial.println("######################################");
}



void setupPins() {
  pinMode(ledState1, OUTPUT);
  pinMode(ledState2, OUTPUT);
  pinMode(relaisPin, OUTPUT);

  pinMode(changeStateBtn, INPUT);
  pinMode(startMillBtn, INPUT);
  pinMode(lowerTimeBtn, INPUT);
  pinMode(raiseTimeBtn, INPUT);

  digitalWrite(changeStateBtn, LOW);
  digitalWrite(startMillBtn, LOW);
  analogWrite(lowerTimeBtn, LOW);
  analogWrite(raiseTimeBtn, LOW);
}


void updateLED() {
  if(stateBtn == 0) {
    digitalWrite(ledState1, LOW);
    digitalWrite(ledState2, HIGH);
  } else if (stateBtn == 1) {
    digitalWrite(ledState1, HIGH);
    digitalWrite(ledState2, LOW);
  } else if (stateBtn == 2) {
    digitalWrite(ledState1, HIGH);
    digitalWrite(ledState2, HIGH);
  }
}

void updateState() {
    updateLED();
}

void lowerTime(int state) {
  if(state == 1) {
    Serial.println("Lowered time for mode 1");
    timeState1 -= 200; 
    Serial.print("New time for mode 1 is: ");
    Serial.println(timeState1);
    saveDataToEEPROM(state);
  }
  else if(state == 2) {
    Serial.println("Lowered time for mode 2");
    timeState2 -= 200; 
    Serial.print("New time for mode 2 is: ");
    Serial.println(timeState2);
    saveDataToEEPROM(state);
  }

  
}

void raiseTime(int state) {
  if(state == 1) {
    Serial.println("Raised time for mode 1");
    timeState1 += 200; 
    Serial.print("New time for mode 1 is: ");
    Serial.println(timeState1);
    saveDataToEEPROM(state);
  }
  else if(state == 2) {
    Serial.println("Raised time for mode 2");
    timeState2 += 200; 
    Serial.print("New time for mode 2 is: ");
    Serial.println(timeState2);
    saveDataToEEPROM(state);
  }

}

void saveDataToEEPROM(int state) {
  // times are set as multiples of 200
  // therefore in one byte we can save up to 20480 ms runtime
  // addr 0 for timestate 1, 2 for timestate 2
  if(state == 1) {
    int val = timeState1/20;
    Serial.print("Saving timeState 1 as: ");
    Serial.println(val);
    writeIntIntoEEPROM(0, val);
  } else if(state == 2) {
    int val = timeState2/20;
    Serial.print("Saving timeState 2 as: ");
    Serial.println(val);
    writeIntIntoEEPROM(2, val);
  }
}

void writeIntIntoEEPROM(int address, int number)
{ 
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}

int readIntFromEEPROM(int address)
{
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}



void loop() {

  int startRead = digitalRead(startMillBtn);
  if(startRead == 1) {
    // start milling
    if(isMilling == false) {
      isMilling = true;
      Serial.print("Machine mode is: ");
      Serial.println((int)stateBtn);
      
      unsigned long start = millis();
      int duration = 250;
      if(stateBtn == 1) { duration = timeState1; }
      else if(stateBtn == 2) { duration = timeState2; }

      Serial.print("Duration is: ");
      Serial.println((int)duration);

      Serial.println("Starting milling...");
      int elapse = 0;
      // run for a specified time, keep the motor running
      while(millis() < start + duration) {
        digitalWrite(relaisPin, HIGH);
        isMilling = true;
        
        // show countdown in automatic mode
       
      }

      // time is over, reset machine
      Serial.println("Milling done. Stopping.");
      digitalWrite(relaisPin, LOW);
      isMilling = false;
      delay(500);
      
    } else if (isMilling == true) {
      Serial.println("Milling already in progress...");
      delay(200);
    }
    
  } else if (startRead == 0 && isMilling == false) {
    // allow changes only when not milling

    // switch modes if button was pressed
    int stateRead = digitalRead(changeStateBtn);  
    if(stateRead == 1) {
      switch (stateBtn) {
        case 0: stateBtn = 1; break;
        case 1: stateBtn = 2; break;
        case 2: stateBtn = 0; break;
      }
      Serial.print("Machine mode changed. Is now: ");
      Serial.println((int)stateBtn);
    }

    // switch modes if button was pressed
    int lowerTimeRead = analogRead(lowerTimeBtn);  
    if(lowerTimeRead > 250) { lowerTime((int)stateBtn); }

    int raiseTimeRead = analogRead(raiseTimeBtn);  
    if(raiseTimeRead > 250) { raiseTime((int)stateBtn); }
    
    updateState();
    delay(200);
  }
  
}
