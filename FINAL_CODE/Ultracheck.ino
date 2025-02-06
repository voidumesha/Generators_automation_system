#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SoftwareSerial.h>

// === Ultrasonic Sensor Pins ===
#define TRIG_PIN 7
#define ECHO_PIN 6

// === 5-Channel Flame Sensor Pins ===
#define FLAME_D1 30
#define FLAME_D2 31
#define FLAME_D3 32
#define FLAME_D4 33
#define FLAME_D5 34

#define FLAME_A1 A0
#define FLAME_A2 A1
#define FLAME_A3 A2
#define FLAME_A4 A3
#define FLAME_A5 A4

// === Generator ON/OFF Button ===
#define GEN_BUTTON 40

// === Fuel Tank Lid Push Button ===
#define LID_BUTTON 41

// === Manual Display Push Button ===
#define DISPLAY_BUTTON 42

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
const int MAX_FUEL_LEVEL_CM = 15; // 2L bottle = 15cm
const int FULL_FUEL_PERCENTAGE = 100;
const int FUEL_THRESHOLDS[] = {100, 75, 50, 25, 0}; // % levels for messages
const float CONSUMPTION_RATE = 200.0;               // 200ml per hour

float previousFuelLevel = 100.0;
unsigned long generatorStartTime = 0;
bool generatorRunning = false;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  sim800l.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(FLAME_D1, INPUT);
  pinMode(FLAME_D2, INPUT);
  pinMode(FLAME_D3, INPUT);
  pinMode(FLAME_D4, INPUT);
  pinMode(FLAME_D5, INPUT);

  pinMode(GEN_BUTTON, INPUT_PULLUP);
  pinMode(LID_BUTTON, INPUT_PULLUP);
  pinMode(DISPLAY_BUTTON, INPUT_PULLUP);

  if (!rtc.begin())
    Serial.println("RTC failed!");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    Serial.println("OLED failed!");

  displayMessage("System Ready!", "");
  sendSMS("Generator Monitoring System Started!");
}

void loop()
{
  updateDisplay();
  checkFuelLevel();
  checkFlameSensor();
  checkGeneratorStatus();
  checkLidStatus();
  checkScheduledFuelReport();
  delay(1000);
}

// === Function to Measure Fuel Level ===
float getFuelLevel()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  float distanceCM = (duration * 0.0343) / 2;

  float fuelPercentage = ((MAX_FUEL_LEVEL_CM - distanceCM) / MAX_FUEL_LEVEL_CM) * 100.0;
  return fuelPercentage >= 0 ? fuelPercentage : 0;
}

void checkFuelLevel()
{
  float fuelPercentage = getFuelLevel();

  if (fuelPercentage != previousFuelLevel)
  {
    for (int i = 0; i < 5; i++)
    {
      if (fuelPercentage <= FUEL_THRESHOLDS[i])
      {
        sendSMS("Fuel Level: " + String(FUEL_THRESHOLDS[i]) + "%");
        break;
      }
    }
    previousFuelLevel = fuelPercentage;
  }
}

// === Function to Detect Fire ===
void checkFlameSensor()
{
  if (!digitalRead(FLAME_D1) || !digitalRead(FLAME_D2) || !digitalRead(FLAME_D3) || !digitalRead(FLAME_D4) || !digitalRead(FLAME_D5))
  {
    sendSMS("Near the bottle fire detected!");
  }
}

// === Function to Detect Generator ON/OFF ===
void checkGeneratorStatus()
{
  bool genState = !digitalRead(GEN_BUTTON);

  if (genState && !generatorRunning)
  {
    generatorRunning = true;
    generatorStartTime = millis();
    sendSMS("Generator Turned ON!");
  }

  if (!genState && generatorRunning)
  {
    generatorRunning = false;
    unsigned long runTime = (millis() - generatorStartTime) / 3600000.0;
    float expectedConsumption = runTime * CONSUMPTION_RATE;
    sendSMS("Generator OFF! Fuel Used: " + String(expectedConsumption) + "ml");
  }
}

// === Function to Detect Fuel Theft (Lid Opened) ===
void checkLidStatus()
{
  if (digitalRead(LID_BUTTON) == LOW)
  {
    sendSMS("WARNING: Fuel Tank Lid Opened!");
  }
}

// === Function to Send Fuel Level at 8AM and 5PM ===
void checkScheduledFuelReport()
{
  DateTime now = rtc.now();
  if ((now.hour() == 8 || now.hour() == 17) && now.minute() == 0)
  {
    checkFuelLevel();
  }
}

// === OLED Display Update ===
void updateDisplay()
{
  DateTime now = rtc.now();
  String dateStr = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day());
  String timeStr = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());

  if (digitalRead(DISPLAY_BUTTON) == LOW)
  {
    float fuelPercentage = getFuelLevel();
    String fuelStatus = "Fuel: " + String(fuelPercentage) + "%";
    String genStatus = generatorRunning ? "Generator: ON" : "Generator: OFF";
    displayMessage(fuelStatus, genStatus);
  }
  else
  {
    displayMessage(dateStr, timeStr);
  }
}

void displayMessage(String top, String bottom)
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println(top);
  display.setCursor(10, 30);
  display.println(bottom);
  display.display();
}

// === Function to Send SMS ===
void sendSMS(String message)
{
  sim800l.println("AT+CMGF=1");
  delay(100);
  sim800l.println("AT+CMGS=\"+94763005528\"");
  delay(100);
  sim800l.print(message);
  delay(100);
  sim800l.write(26);
    delay(3000);
}