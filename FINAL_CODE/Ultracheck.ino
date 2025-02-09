#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

// === Ultrasonic Sensor Pins ===
#define TRIG_PIN 9
#define ECHO_PIN 10

// === 5-Channel Flame Sensor Pins ===
#define FLAME_D1 30
#define FLAME_D2 31
#define FLAME_D3 32
#define FLAME_D4 33
#define FLAME_D5 34
#define FLAME_ANALOG A0

// === Generator ON/OFF Button ===
#define GEN_BUTTON 40

// === Fuel Tank Lid Push Button ===
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
SoftwareSerial sim800l(SIM_TX, SIM_RX);

// === Constants ===
const float MAX_FUEL_LEVEL_CM = 15.0;    // Tank height in cm
const float MAX_FUEL_VOLUME_ML = 1500.0; // Tank capacity in mL
const float CONSUMPTION_RATE = 200.0;    // mL per hour
const int SAMPLES = 5;                   // Number of samples for averaging
const String PHONE_NUMBER = "+94763005528";

// === Variables ===
float previousFuelLevel = MAX_FUEL_VOLUME_ML;
unsigned long generatorStartTime = 0;
bool generatorRunning = false;
bool displayMode = false;
bool fireDetected = false;
bool lidOpened = false;
unsigned long lastReportTime = 0;
float fuelLevelReadings[SAMPLES];
int readingIndex = 0;

void setup()
{
  Serial.begin(9600);
  Wire.begin();

  // Initialize GSM module
  sim800l.begin(9600);
  initializeGSM();

  // Initialize pins
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
  pinMode(GEN_SWITCH, INPUT_PULLUP);

  Serial.println("✅ System Initializing...");

  // Initialize RTC
  if (!rtc.begin())
  {
    Serial.println("❌ RTC module failed!");
  }
  else
  {
    if (rtc.lostPower())
    {
      Serial.println("⚠ RTC lost power, setting current time...");
      rtc.adjust(DateTime(__DATE__, __TIME__));
    }
  }

  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("❌ OLED initialization failed!");
  }

  // Initialize fuel level readings array
  for (int i = 0; i < SAMPLES; i++)
  {
    fuelLevelReadings[i] = 0;
  }

  Serial.println("✅ System Ready!");
  displayMessage("System Ready!", "");
  sendSMS("Generator Monitoring System Started!");
}

void initializeGSM()
{
  delay(3000);           // Give module time to initialize
  sim800l.println("AT"); // Check module
  delay(1000);
  sim800l.println("AT+CFUN=1"); // Set full functionality
  delay(1000);
  sim800l.println("AT+CMGF=1"); // Set SMS text mode
  delay(1000);
  sim800l.println("AT+CNMI=2,2,0,0,0"); // Set how newly arrived SMS should be handled
  delay(1000);

  // Wait for network registration
  sim800l.println("AT+CREG?");
  delay(1000);

  // Delete all SMS to free up memory
  sim800l.println("AT+CMGD=1,4");
  delay(1000);
}

float getFuelLevel()
{
  float distance = 0;

  // Take multiple readings
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 100000);

  if (duration == 0)
  {
    Serial.println("⚠ Ultrasonic Sensor Timeout!");
    return previousFuelLevel;
  }

  // Calculate distance in cm
  distance = (duration * 0.0343) / 2;

  // Constrain distance reading
  distance = constrain(distance, 0, MAX_FUEL_LEVEL_CM);

  // Calculate fuel volume (ml)
  float fuelVolume = ((MAX_FUEL_LEVEL_CM - distance) / MAX_FUEL_LEVEL_CM) * MAX_FUEL_VOLUME_ML;

  // Add to rolling average
  fuelLevelReadings[readingIndex] = fuelVolume;
  readingIndex = (readingIndex + 1) % SAMPLES;

  // Calculate average
  float avgFuelLevel = 0;
  for (int i = 0; i < SAMPLES; i++)
  {
    avgFuelLevel += fuelLevelReadings[i];
  }
  avgFuelLevel /= SAMPLES;

  return constrain(avgFuelLevel, 0, MAX_FUEL_VOLUME_ML);
}

void checkFuelLevel()
{
  float currentFuelLevel = getFuelLevel();

  // Only update if change is significant (more than 2%)
  if (abs(currentFuelLevel - previousFuelLevel) > (MAX_FUEL_VOLUME_ML * 0.02))
  {
    previousFuelLevel = currentFuelLevel;
    Serial.print("📊 Fuel Level: ");
    Serial.print(currentFuelLevel);
    Serial.println(" mL");
  }
}

void checkDisplayButton()
{
  if (digitalRead(DISPLAY_BUTTON) == LOW)
  {
    displayMode = !displayMode;
    delay(500);
  }
}

void updateDisplay()
{
  DateTime now = rtc.now();
  if (!displayMode)
  {
    String dateStr = String(now.year()) + "/" +
                     (now.month() < 10 ? "0" : "") + String(now.month()) + "/" +
                     (now.day() < 10 ? "0" : "") + String(now.day());
    String timeStr = (now.hour() < 10 ? "0" : "") + String(now.hour()) + ":" +
                     (now.minute() < 10 ? "0" : "") + String(now.minute());
    displayMessage(dateStr, timeStr);
  }
  else
  {
    String fuelStr = "Fuel:" + String(int(getFuelLevel())) + "mL";
    String genStr = digitalRead(GEN_SWITCH) ? "Gen:ON" : "Gen:OFF";
    displayMessage(fuelStr, genStr);
  }
}

void displayMessage(String top, String bottom)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println(top);
  display.setCursor(0, 40);
  display.println(bottom);
  display.display();
}

void checkFlameSensor()
{
  int flameAnalog = analogRead(FLAME_ANALOG);
  bool flameDigital = !digitalRead(FLAME_D1) || !digitalRead(FLAME_D2) ||
                      !digitalRead(FLAME_D3) || !digitalRead(FLAME_D4) ||
                      !digitalRead(FLAME_D5);

  if ((flameAnalog < 500 || flameDigital) && !fireDetected)
  {
    sendSMS("ALERT: Fire detected near generator!");
    fireDetected = true;
  }
  else if (flameAnalog >= 500 && !flameDigital)
  {
    fireDetected = false;
  }
}

void checkGeneratorStatus()
{
  bool genButtonState = !digitalRead(GEN_BUTTON);
  bool genSwitchState = !digitalRead(GEN_SWITCH);

  // Check Generator Button
  if (genButtonState && !generatorRunning)
  {
    generatorRunning = true;
    generatorStartTime = millis();
    sendSMS("Generator started manually!");
  }
  else if (!genButtonState && generatorRunning)
  {
    generatorRunning = false;
    float runTime = (millis() - generatorStartTime) / 3600000.0; // Convert to hours
    float fuelUsed = runTime * CONSUMPTION_RATE;
    sendSMS("Generator stopped. Run time: " + String(runTime, 1) +
            "hrs, Est. fuel used: " + String(fuelUsed, 0) + "mL");
  }

  // Check Generator Switch
  static bool lastSwitchState = false;
  if (genSwitchState != lastSwitchState)
  {
    if (genSwitchState)
    {
      sendSMS("Generator switch turned ON");
    }
    lastSwitchState = genSwitchState;
  }
}

void checkLidStatus()
{
  static bool lastLidState = true;
  bool currentLidState = digitalRead(LID_BUTTON);

  if (currentLidState != lastLidState)
  {
    if (currentLidState == LOW)
    {
      sendSMS("WARNING: Fuel tank lid opened!");
    }
    lastLidState = currentLidState;
    delay(50); // Debounce
  }
}

void checkScheduledFuelReport()
{
  DateTime now = rtc.now();

  // Check if it's 8 AM or 5 PM
  if ((now.hour() == 8 || now.hour() == 17) && now.minute() == 0)
  {
    // Prevent multiple reports in the same minute
    if (millis() - lastReportTime > 60000)
    {
      String fuelReport = "Scheduled Report - Fuel Level: " +
                          String(getFuelLevel(), 0) + "mL";
      sendSMS(fuelReport);
      lastReportTime = millis();
    }
  }
}

void checkGSMCommands()
{
  while (sim800l.available())
  {
    String message = sim800l.readString();

    // Check if it's an SMS message
    if (message.indexOf("+CMT:") > -1)
    {
      // Wait a bit for the full message
      delay(100);

      // If message contains "fuel" (case insensitive check)
      if (message.indexOf("fuel") > -1 || message.indexOf("FUEL") > -1)
      {
        String response = "Current fuel level: " + String(int(getFuelLevel())) + "mL";
        sendSMS(response);
      }
    }
  }
}

void sendSMS(String message)
{
  Serial.println("📩 Sending SMS: " + message);

  sim800l.println("AT+CMGF=1"); // Text mode
  delay(500);

  sim800l.println("AT+CMGS=\"" + PHONE_NUMBER + "\"");
  delay(500);

  sim800l.print(message);
  delay(100);

  sim800l.write(0x1A); // Ctrl+Z to send
  delay(3000);         // Wait for message to be sent
}

void loop()
{
  checkDisplayButton();
  updateDisplay();
  checkFuelLevel();
  checkFlameSensor();
  checkGeneratorStatus();
  checkLidStatus();
  checkScheduledFuelReport();
  checkGSMCommands();
  delay(1000);
}