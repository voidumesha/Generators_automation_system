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
#define FLAME_ANALOG A0 // Using Analog input for better accuracy

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
const float MAX_FUEL_LEVEL_CM = 15.0;
const float MAX_FUEL_VOLUME_ML = 1500.0;
const float CONSUMPTION_RATE = 200.0;

float previousFuelLevel = MAX_FUEL_VOLUME_ML;
unsigned long generatorStartTime = 0;
bool generatorRunning = false;
bool displayMode = false;
bool fireDetected = false;

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
  pinMode(FLAME_ANALOG, INPUT);

  pinMode(GEN_BUTTON, INPUT_PULLUP);
  pinMode(LID_BUTTON, INPUT_PULLUP);
  pinMode(DISPLAY_BUTTON, INPUT_PULLUP);
  pinMode(GEN_SWITCH, INPUT_PULLUP);

  Serial.println("✅ System Initializing...");

  if (!rtc.begin())
  {
    Serial.println("❌ RTC module failed!");
  }
  else
  {
    if (rtc.lostPower())
    {
      Serial.println("⚠ RTC lost power, setting current time...");
      rtc.adjust(DateTime(_DATE, __TIME_));
    }
  }

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println("❌ OLED initialization failed!");
  }

  Serial.println("✅ System Ready!");
  displayMessage("System Ready!", "");
  sendSMS("Generator Monitoring System Started!");
}

void loop()
{
  checkDisplayButton();
  updateDisplay();
  checkFuelLevel(); // ✅ Now the function is properly defined!
  checkFlameSensor();
  checkGeneratorStatus();
  checkLidStatus();
  checkScheduledFuelReport();
  delay(1000);
}

// === Function to Toggle OLED Display Mode ===
void checkDisplayButton()
{
  if (digitalRead(DISPLAY_BUTTON) == LOW)
  {
    displayMode = !displayMode;
    Serial.println("📟 Display mode changed.");
    delay(500);
  }
}

// === Function to Update OLED Display ===
void updateDisplay()
{
  DateTime now = rtc.now();
  if (!displayMode)
  {
    String dateStr = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day());
    String timeStr = String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second());
    displayMessage(dateStr, timeStr);
  }
  else
  {
    float fuelVolumeML = getFuelLevel();
    bool genStatus = !digitalRead(GEN_SWITCH);
    String fuelStatus = "Fuel: " + String(fuelVolumeML) + "mL";
    String genState = genStatus ? "Gen: ON" : "Gen: OFF";
    displayMessage(fuelStatus, genState);
  }
}

// === Function to Display Message on OLED ===
void displayMessage(String top, String bottom)
{
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println(top);
  display.setCursor(10, 40);
  display.println(bottom);
  display.display();
}

// === Function to Measure Fuel Level ===
void checkFuelLevel()
{
  float fuelVolumeML = getFuelLevel();
  Serial.print("📊 Fuel Level: ");
  Serial.print(fuelVolumeML);
  Serial.println(" mL");

  if (fuelVolumeML != previousFuelLevel)
  {
    sendSMS("Fuel Level: " + String(fuelVolumeML) + "mL");
    previousFuelLevel = fuelVolumeML;
  }
}

// === Function to Get Fuel Level from Ultrasonic Sensor ===
float getFuelLevel()
{
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0)
  {
    Serial.println("⚠ Ultrasonic Sensor Timeout! Check connections.");
    return previousFuelLevel;
  }

  float distanceCM = (duration * 0.0343) / 2;
  float fuelVolumeML = ((MAX_FUEL_LEVEL_CM - distanceCM) / MAX_FUEL_LEVEL_CM) * MAX_FUEL_VOLUME_ML;
  return fuelVolumeML >= 0 ? fuelVolumeML : 0;
}

// === Function to Detect Fire ===
void checkFlameSensor()
{
  int flameValue = analogRead(FLAME_ANALOG);
  Serial.print("🔥 Flame Sensor Value: ");
  Serial.println(flameValue);

  if (flameValue < 500 && !fireDetected)
  {
    Serial.println("🔥 Fire detected near the generator!");
    sendSMS("Fire detected near the bottle!");
    fireDetected = true;
  }
  else if (flameValue >= 500)
  {
    fireDetected = false;
  }
}

// === Function to Check Generator ON/OFF Status ===
void checkGeneratorStatus()
{
  bool genState = !digitalRead(GEN_BUTTON);

  if (genState && !generatorRunning)
  {
    generatorRunning = true;
    generatorStartTime = millis();
    Serial.println("✅ Generator Turned ON!");
    sendSMS("Generator Turned ON!");
    sendSMS("Generator Powered!");
  }

  if (!genState && generatorRunning)
  {
    generatorRunning = false;
    unsigned long runTime = (millis() - generatorStartTime) / 3600000.0;
    float expectedConsumption = runTime * CONSUMPTION_RATE;
    Serial.print("❌ Generator Turned OFF! Fuel Used: ");
    Serial.print(expectedConsumption);
    Serial.println(" mL");
    sendSMS("Generator OFF! Fuel Used: " + String(expectedConsumption) + "mL");
  }
}

// === Function to Detect Fuel Theft ===
void checkLidStatus()
{
  if (digitalRead(LID_BUTTON) == LOW)
  {
    Serial.println("⚠ WARNING: Fuel Tank Lid Opened!");
    sendSMS("WARNING: Fuel Tank Lid Opened!");
  }
}

// === Function to Send Scheduled Fuel Reports ===
void checkScheduledFuelReport()
{
  DateTime now = rtc.now();
  if ((now.hour() == 8 || now.hour() == 17) && now.minute() == 0)
  {
    Serial.println("📅 Sending Scheduled Fuel Report...");
    checkFuelLevel();
  }
}

// === Function to Send SMS ===
void sendSMS(String message)
{
  Serial.print("📩 Sending SMS: ");
  Serial.println(message);

  sim800l.println("AT+CMGF=1");
  delay(100);
  sim800l.println("AT+CMGS=\"+94763005528\"");
  delay(100);
  sim800l.print(message);
  delay(100);
  sim800l.write(26);
    delay(3000);
}