#include <EEPROM.h>

// ---- Pins ----
const int ledState1     = 2;
const int ledState2     = 4;
const int relaisPin     = 12;
const int changeStateBtn = 10;
const int startMillBtn  = 9;
const int lowerTimeBtn  = 8;
const int raiseTimeBtn  = 6;

// ---- Konstanten ----
const int STEP_MS          = 200;   // Schrittweite beim Verstellen
const int MIN_DURATION_MS  = 200;   // Untergrenze -> verhindert negative/0 Laufzeiten
const int MAX_DURATION_MS  = 30000; // Obergrenze -> plausibler Max-Wert (30s)
const unsigned long ABSOLUTE_MAX_RUNTIME = 30000UL; // Failsafe, UNABHÄNGIG von duration
const unsigned long DEBOUNCE_MS = 300; // groesser = "traeger", aber Mehrfachtrigger pro Klick werden verhindert

// ---- Zustand ----
int stateBtn = 0;
long timeState1 = 0;
long timeState2 = 0;
bool isMilling = false;

// Entprellung pro Taster
unsigned long lastDebounce_changeState = 0;
unsigned long lastDebounce_lowerTime   = 0;
unsigned long lastDebounce_raiseTime   = 0;

void setup() {
  setupPins();
  Serial.begin(9600);

  stateBtn = 2;

  timeState1 = (long)readIntFromEEPROM(0) * 20;
  timeState2 = (long)readIntFromEEPROM(2) * 20;

  if (timeState1 <= 0) { timeState1 = 4000; }
  if (timeState2 <= 0) { timeState2 = 8000; }
  timeState1 = clampDuration(timeState1);
  timeState2 = clampDuration(timeState2);

  updateState();

  Serial.println(F("######################################"));
  Serial.println(F("Machine started."));
  Serial.println(F("Available modes:"));
  Serial.println(F("0 - manual"));
  Serial.println(F("1 - single shot (timeState1)"));
  Serial.println(F("2 - double shot (timeState2)"));
  Serial.print(F("Machine mode is now (by default): "));
  Serial.println(stateBtn);
  Serial.println(F("Saved timeState values are:"));
  Serial.print(F("timeState1: "));
  Serial.println(timeState1);
  Serial.print(F("timeState2: "));
  Serial.println(timeState2);
  Serial.println(F("######################################"));
}

void setupPins() {
  pinMode(ledState1, OUTPUT);
  pinMode(ledState2, OUTPUT);
  pinMode(relaisPin, OUTPUT);

  // INPUT_PULLUP statt manuellem digitalWrite(HIGH) nach pinMode(INPUT)
  pinMode(changeStateBtn, INPUT_PULLUP);
  pinMode(startMillBtn, INPUT_PULLUP);
  pinMode(lowerTimeBtn, INPUT_PULLUP);
  pinMode(raiseTimeBtn, INPUT_PULLUP);

  digitalWrite(relaisPin, LOW);
}

void updateLED() {
  if (stateBtn == 0) {
    digitalWrite(ledState1, LOW);
    digitalWrite(ledState2, LOW);
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

// Sorgt dafür, dass die Laufzeit NIE negativ oder unplausibel groß werden kann.
// Das ist der zentrale Fix für die Hänger.
long clampDuration(long value) {
  if (value < MIN_DURATION_MS) return MIN_DURATION_MS;
  if (value > MAX_DURATION_MS) return MAX_DURATION_MS;
  return value;
}

void lowerTime(int state) {
  if (state == 1) {
    timeState1 = clampDuration(timeState1 - STEP_MS);
    Serial.print(F("Lowered time for mode 1. New time: "));
    Serial.println(timeState1);
    saveDataToEEPROM(state);
  } else if (state == 2) {
    timeState2 = clampDuration(timeState2 - STEP_MS);
    Serial.print(F("Lowered time for mode 2. New time: "));
    Serial.println(timeState2);
    saveDataToEEPROM(state);
  }
}

void raiseTime(int state) {
  if (state == 1) {
    timeState1 = clampDuration(timeState1 + STEP_MS);
    Serial.print(F("Raised time for mode 1. New time: "));
    Serial.println(timeState1);
    saveDataToEEPROM(state);
  } else if (state == 2) {
    timeState2 = clampDuration(timeState2 + STEP_MS);
    Serial.print(F("Raised time for mode 2. New time: "));
    Serial.println(timeState2);
    saveDataToEEPROM(state);
  }
}

void saveDataToEEPROM(int state) {
  if (state == 1) {
    int val = timeState1 / 20;
    writeIntIntoEEPROM(0, val);
  } else if (state == 2) {
    int val = timeState2 / 20;
    writeIntIntoEEPROM(2, val);
  }
}

void writeIntIntoEEPROM(int address, int number) {
  byte byte1 = number >> 8;
  byte byte2 = number & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}

int readIntFromEEPROM(int address) {
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

// Führt den Mahlvorgang für eine feste Dauer aus.
// duration ist durch clampDuration() garantiert positiv & begrenzt,
// zusätzlich gibt es einen fixen, von duration unabhängigen Failsafe.
void runTimedMilling(unsigned long duration) {
  Serial.print(F("Starting milling, duration: "));
  Serial.println(duration);

  unsigned long start = millis();
  unsigned long lastPrint = start;
  bool timeout = false;

  digitalWrite(relaisPin, HIGH);
  isMilling = true;

  while (millis() - start < duration) {
    unsigned long elapsedTotal = millis() - start;

    // Fixer Failsafe, unabhängig vom (bereits geclampten) duration-Wert
    if (elapsedTotal > ABSOLUTE_MAX_RUNTIME) {
      Serial.println(F("SAFETY STOP: absolute max runtime reached!"));
      timeout = true;
      break;
    }

    // Countdown max. 1x pro Sekunde ausgeben (statt Serial-Flut)
    if (millis() - lastPrint >= 1000) {
      lastPrint = millis();
      long remaining = (long)duration - (long)elapsedTotal;
      Serial.println(remaining);
    }
  }

  digitalWrite(relaisPin, LOW);
  isMilling = false;

  if (timeout) {
    Serial.println(F("Milling stopped by safety timeout."));
  } else {
    Serial.println(F("Milling done. Stopping."));
  }
  delay(300);
}

void runManualMilling() {
  Serial.println(F("Starting manual milling..."));
  unsigned long start = millis();
  digitalWrite(relaisPin, HIGH);
  isMilling = true;

  while (digitalRead(startMillBtn) == LOW) {
    // Auch im manuellen Modus nicht grenzenlos laufen lassen
    if (millis() - start > ABSOLUTE_MAX_RUNTIME) {
      Serial.println(F("SAFETY STOP: manual mode max runtime reached!"));
      break;
    }
    delay(5);
  }

  digitalWrite(relaisPin, LOW);
  isMilling = false;
  Serial.println(F("Milling done. Stopping."));
}

void loop() {
  int startRead = digitalRead(startMillBtn);

  if (startRead == LOW) {
    if (!isMilling) {
      Serial.print(F("Machine mode is: "));
      Serial.println(stateBtn);

      if (stateBtn == 1) {
        runTimedMilling((unsigned long)timeState1);
      } else if (stateBtn == 2) {
        runTimedMilling((unsigned long)timeState2);
      } else if (stateBtn == 0) {
        runManualMilling();
      }
    } else {
      // Sollte praktisch nie erreicht werden, da runTimedMilling/runManualMilling blockieren
      Serial.println(F("Milling already in progress..."));
      delay(200);
    }
  } else if (!isMilling) {
    // Änderungen nur erlauben, wenn gerade nicht gemahlen wird
    unsigned long now = millis();

    if (digitalRead(changeStateBtn) == LOW &&
        (now - lastDebounce_changeState) > DEBOUNCE_MS) {
      lastDebounce_changeState = now;
      switch (stateBtn) {
        case 0: stateBtn = 1; break;
        case 1: stateBtn = 2; break;
        case 2: stateBtn = 0; break;
      }
      Serial.print(F("Machine mode changed. Is now: "));
      Serial.println(stateBtn);
    }

    if (digitalRead(lowerTimeBtn) == LOW &&
        (now - lastDebounce_lowerTime) > DEBOUNCE_MS) {
      lastDebounce_lowerTime = now;
      lowerTime(stateBtn);
    }

    if (digitalRead(raiseTimeBtn) == LOW &&
        (now - lastDebounce_raiseTime) > DEBOUNCE_MS) {
      lastDebounce_raiseTime = now;
      raiseTime(stateBtn);
    }

    updateState();
  }
}
