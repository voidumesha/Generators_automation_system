#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>


#define TRIG_PIN 9
#define ECHO_PIN 10 

#define FLAME_D1 30
#define FLAME_D2 31
#define FLAME_D3 32
#define FLAME_D4 33
#define FLAME_D5 34
#define FLAME_ANALOG A0 
#define FLAME_ANALOG A2
#define FLAME_ANALOG A2
#define FLAME_ANALOG A3
#define FLAME_ANALOG A4


// === Generator ON/OFF Button ===
#define GEN_BUTTON 40

#define LID_BUTTON 41
// === Manual Display Toggle Button ===
#define DISPLAY_BUTTON 43  

// === Generator On/Off Switch ===
#define GEN_SWITCH 44  

// === OLED Display ===
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// === RTC Module ===
RTC_DS3231 rtc;

// === GSM Module (SIM800L) ===
#define SIM_TX 18
#define SIM_RX 19


// === Constants ===
const float MAX_FUEL_LEVEL_CM = 11.0;
const float MAX_FUEL_VOLUME_ML = 1500.0;
const float CONSUMPTION_RATE = 200.0;

float previousFuelLevel = MAX_FUEL_VOLUME_ML;
int lastSentFuelLevel = -1;
unsigned long generatorStartTime = 0;
bool generatorRunning = false;
bool displayMode = false;
bool fireDetected = false;
bool lidOpened = false;
unsigned long systemStartTime = millis();  // Track system startup time
int fireTriggerCount = 0;

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  delay(1000);


  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  pinMode(FLAME_D1, INPUT);
  pinMode(FLAME_D2, INPUT);
  pinMode(FLAME_D3, INPUT);
  pinMode(FLAME_D4, INPUT);
  pinMode(FLAME_D5, INPUT);
  pinMode(FLAME_ANALOG, INPUT);
  
  
  pinMode(GEN_BUTTON, INPUT_PULLUP);
  pinMode(LID_BUTTON, INPUT_PULLUP);
  pinMode(DISPLAY_BUTTON, INPUT_PULLUP);
  

  Serial.println("✅ System Initializing...");


  // 🔵 Read flame sensor values before starting
  Serial.println("🔥 Calibrating Flame Sensor...");
  int flameAnalog1 = analogRead(A0);
  int flameAnalog2 = analogRead(A1);
  int flameAnalog3 = analogRead(A2);
  int flameAnalog4 = analogRead(A3);
  int flameAnalog5 = analogRead(A4);

  Serial.print("Initial Flame Sensor Values: ");
  Serial.print(flameAnalog1); Serial.print(", ");
  Serial.print(flameAnalog2); Serial.print(", ");
  Serial.print(flameAnalog3); Serial.print(", ");
  Serial.print(flameAnalog4); Serial.print(", ");
  Serial.println(flameAnalog5);

  delay(3000); 

  if (!rtc.begin()) {
    Serial.println("❌ RTC module failed!");
  } else {
    if (rtc.lostPower()) {
      Serial.println("⚠ RTC lost power, setting current time...");
      rtc.adjust(DateTime(_DATE, __TIME_));
    }
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("❌ OLED initialization failed!");
  }

  Serial.println("✅ System Ready!");
  displayMessage("System Ready!", "");
  sendSMS("Generator Monitoring System Started!");
}

void loop() {
  checkDisplayButton();
  updateDisplay();
  checkGeneratorStatus();
  checkFuelLevel();
  checkFlameSensor();
  checkLidStatus();
  checkScheduledFuelReport();
  checkGSMCommands();

  delay(1000);
}

float getFuelLevel() {
  // 🔴 If the lid is open, keep returning the last known fuel level
  if (lidOpened) {
    Serial.println("⚠ Lid is open! Pausing fuel level measurement.");
    return previousFuelLevel;  // Do not take new readings
  }

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 100000);
  if (duration == 0) {
    Serial.println("⛔ Ultrasonic Sensor Timeout! Check connections.");
    return previousFuelLevel;
  }

  float distanceCM = (duration * 0.0343) / 2;
  float fuelVolumeML = ((MAX_FUEL_LEVEL_CM - distanceCM) / MAX_FUEL_LEVEL_CM) * MAX_FUEL_VOLUME_ML;

  // ✅ Update only when the lid is closed
  previousFuelLevel = constrain(fuelVolumeML, 0, MAX_FUEL_VOLUME_ML);
  return previousFuelLevel;
}


void checkFuelLevel() {
  // 🔴 If the lid is open, DO NOT check fuel level
  if (lidOpened) {
    Serial.println("⚠ Lid is open! Skipping fuel level check.");
    return;  // 🔴 Exit function immediately
  }

  float fuelVolumeML = getFuelLevel();
  Serial.print("📊 Fuel Level: ");
  Serial.print(fuelVolumeML);
  Serial.println(" mL");

  // Convert fuel level to percentage
  float fuelPercentage = (fuelVolumeML / MAX_FUEL_VOLUME_ML) * 100;

  // ✅ Only send SMS when lid is closed
  if (fuelPercentage <= 0 && lastSentFuelLevel != 0) {
    sendSMS("Fuel level is 0%! Please refill the generator.");
    lastSentFuelLevel = 0;
  }
  else if (fuelPercentage <= 25 && lastSentFuelLevel != 25) {
    sendSMS("Fuel level is 25%. Please refill soon.");
    lastSentFuelLevel = 25;
  }
  else if (fuelPercentage <= 50 && lastSentFuelLevel != 50) {
    sendSMS("Fuel level is 50%. Half full, consider refilling.");
    lastSentFuelLevel = 50;
  }
  else if (fuelPercentage == 100 && lastSentFuelLevel != 100) {
    sendSMS("Fuel level is at 100%. Full tank.");
    lastSentFuelLevel = 100;
  }

  // ✅ Update fuel level tracking only when lid is closed
  if (abs(fuelVolumeML - previousFuelLevel) > 10) {
    previousFuelLevel = fuelVolumeML;
  }
}







// === Function to Toggle OLED Display Mode ===
void checkDisplayButton() {
  if (digitalRead(DISPLAY_BUTTON) == LOW) {
    displayMode = !displayMode;
    Serial.println("📟 Display mode changed.");
    delay(500);
  }
}

// === Function to Update OLED Display ===
void updateDisplay() {
  DateTime now = rtc.now();
  if (!displayMode) {
    displayMessage(String(now.year()) + "/" + String(now.month()) + "/" + String(now.day()), 
                   String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()));
  } else {
    displayMessage("Fuel: " + String(getFuelLevel()) + "mL", 
                   digitalRead(GEN_BUTTON) ? "Gen: OFF" : "Gen: ON");
  }
}

// === Function to Display Message on OLED ===
void displayMessage(String top, String bottom) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println(top);
  display.setCursor(10, 40);
  display.println(bottom);
  display.display();
}




void checkFlameSensor() {
  // 🔴 Ignore fire detection for the first 10 seconds after startup
  if (millis() - systemStartTime < 10000) {
    Serial.println("⏳ Ignoring fire detection for first 10 seconds...");
    return;
  }

  int flameAnalog1 = analogRead(A0);
  int flameAnalog2 = analogRead(A1);
  int flameAnalog3 = analogRead(A2);
  int flameAnalog4 = analogRead(A3);
  int flameAnalog5 = analogRead(A4);

  bool flameDigital = !digitalRead(FLAME_D1) || !digitalRead(FLAME_D2) || 
                      !digitalRead(FLAME_D3) || !digitalRead(FLAME_D4) || 
                      !digitalRead(FLAME_D5);

  // 🔵 Use an average threshold to avoid false positives
  int avgAnalogValue = (flameAnalog1 + flameAnalog2 + flameAnalog3 + flameAnalog4 + flameAnalog5) / 5;

  Serial.print("🔥 Flame Sensor Readings: ");
  Serial.print(flameAnalog1); Serial.print(", ");
  Serial.print(flameAnalog2); Serial.print(", ");
  Serial.print(flameAnalog3); Serial.print(", ");
  Serial.print(flameAnalog4); Serial.print(", ");
  Serial.print(flameAnalog5);
  Serial.print(" | Average: "); Serial.println(avgAnalogValue);

  // ✅ Require at least 3 consecutive detections before confirming fire
  if (( 10 < avgAnalogValue < 500 || flameDigital)) {
    fireTriggerCount++;  // 🔼 Increase counter if fire detected
  } else {
    fireTriggerCount = 0;  // 🔄 Reset if no fire detected
  }

  // 🔥 Only send alert if fire is detected 3 times in a row
  if (fireTriggerCount >= 3 && !fireDetected) {
    Serial.println("🔥 Fire detected near the generator!");
    sendSMS("ALERT! Fire detected near the generator!");
    fireDetected = true;
  } else if (avgAnalogValue >= 500 && !flameDigital) {
    fireDetected = false;
  }
}





void checkGeneratorStatus() {
  Serial.begin(9600);
  Serial1.begin(9600);
  static bool lastGenState = HIGH;  // Track previous button state
  bool genState = digitalRead(GEN_BUTTON) == LOW;  // Active LOW button

  if (genState && !lastGenState) {  // Detect button press (state change)
    generatorRunning = !generatorRunning;  // Toggle generator state
    
    if (generatorRunning) {
      generatorStartTime = millis();
      sendSMS("Generator Turned ON!");
      Serial.println("⚡ Generator Turned ON!");
    } else {
      float fuelUsed = ((millis() - generatorStartTime) / 3600000.0) * CONSUMPTION_RATE;
      sendSMS("Generator OFF! Fuel Used: " + String(fuelUsed) + "mL"+ " "+ "GOOD");
      Serial.println("Generator OFF! Fuel Used: " + String(fuelUsed) + "mL" + "GOOD");
    }
  }

  lastGenState = genState;  // Update last state
}


// === Function to Check Lid Status ===
void checkLidStatus() {
  if (digitalRead(LID_BUTTON) == LOW && !lidOpened) {
    sendSMS("WARNING: Fuel Tank Lid Opened!");
    lidOpened = true;
  } else if (digitalRead(LID_BUTTON) == HIGH && lidOpened) {
    Serial.println("✅ Lid Closed! Resuming Fuel Measurement.");
    sendSMS("Lid closed. Fuel measurement resumed.");
    lidOpened = false;
  }
}


// === Function to Send Scheduled Fuel Report ===
void checkScheduledFuelReport() {
  DateTime now = rtc.now();
  if ((now.hour() == 8 || now.hour() == 17) && now.minute() == 0) {
    sendSMS("Fuel Level: " + String(getFuelLevel()) + "mL");
  }
}

void checkGSMCommands() {
  if (Serial1.available()) {
    String command = "";
    while (Serial1.available()) {
      char c = Serial1.read();
      if (c == '\n' || c == '\r') break;  // Stop at newline or carriage return
      command += c;
    }
    command.trim();

    Serial.println("📩 Received Command: " + command);

    // ✅ Check if the message is an SMS notification
    if (command.startsWith("+CMTI:")) {
      int indexStart = command.lastIndexOf(",") + 1;
      String indexStr = command.substring(indexStart);
      int msgIndex = indexStr.toInt();  // Convert to integer

      Serial.println("📩 New SMS received at index: " + String(msgIndex));

      // ✅ Read the SMS at the received index
      readSMS(msgIndex);
    }
  }
}


void readSMS(int index) {
  Serial1.print("AT+CMGR=");
  Serial1.println(index);
  delay(1000);

  Serial.println("📩 Reading SMS at index: " + String(index));

  String smsContent = "";
  while (Serial1.available()) {
    String line = Serial1.readStringUntil('\n');
    line.trim();
    Serial.println("📩 " + line); // Debugging output

    // Skip headers and extract the message content
    if (!line.startsWith("+CMGR:") && line.length() > 0) {
      smsContent += line + " ";
    }
  }

  smsContent.trim();
  Serial.println("📩 SMS Content: " + smsContent);

  // ✅ Always send the fuel level when any SMS is received
  float fuelLevel = getFuelLevel();
  String fuelMessage = "Current Fuel Level: " + String(fuelLevel) + " mL";
  sendSMS(fuelMessage);  // Send response

  Serial.println("✅ Sent fuel level to default number.");

  // ✅ Delete the message after reading to prevent storage issues
  Serial1.print("AT+CMGD=");
  Serial1.println(index);
  delay(1000);
}


void sendSMS(String message) {
  Serial.println("📩 Sending SMS: " + message);
  
  Serial1.println("AT");
  delay(1000);
  Serial1.println("AT+CMGF=1"); // Set SMS mode to text
  delay(1000);
  Serial1.println("AT+CMGS=\"+94763005528\""); // Replace with your number
  delay(1000);
  Serial1.print(message);
  delay(500);
  Serial1.write(26); // Send Ctrl+Z to send SMS
  delay(5000);

  Serial.println("📩 SMS Sent!");
}