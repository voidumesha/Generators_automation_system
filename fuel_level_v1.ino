#include <Cth.h>
#include <Wire.h>
#include <DS3231.h>
#include <Adafruit_SSD1306.h>
bool one_time;
DS3231 clock;
RTCDateTime dt;

#define push_button 4                 // push button pin
#define genarator_status 3            // genarator status pin
#define SCREEN_WIDTH 128               // OLED display width, in pixels
#define SCREEN_HEIGHT 64               // OLED display height, in pixels
#define trigPin 5                      // ultrasonic sensor triger pin
#define echoPin 6                      // ultrasonic sensor echo pin
#define first_hour 8                   // first time for sending SMS
#define height 28                      // height of fule tank
int fuel_level = 10;                   // sms sending fuel level
String phoneNumber = "+94719698129";   // phone number 1
String phoneNumber2 = "+94711149536";  // phone number 2

long duration;
float distance;
int level, hr, manu, min, flr;
String CellNumtemp, CellNum;

String textMessage;
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
boolean one_time_sms, low, fl;
void setup() {
  Scheduler.startLoop(button);
  Serial1.begin(9600);
  Scheduler.delay(1000);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Scheduler.startLoop(SMS);
  pinMode(push_button, INPUT_PULLUP);
  pinMode(genarator_status, INPUT_PULLUP);
  clock.begin();
  // Set sketch compiling time
  //clock.setDateTime(__DATE__, __TIME__);               // remove this comment line for update time of RTC
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D for 128x64
    for (;;)
      ;  // Don't proceed, loop forever
  }
  // Clear the buffer
  display.clearDisplay();
  display.display();
  //sms_read();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  dt = clock.getDateTime();
  min = dt.minute;
  hr = dt.hour;

  if (hr == first_hour && min == 00) {
    manu = 3;
    send_sms();
    Scheduler.delay(60000);
  } else {
    Scheduler.delay(5000);
  }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void button(void) {

  if (digitalRead(push_button) == LOW) {
    manu = !manu;
    Scheduler.delay(300);
  }
  if (manu == 1) {
    display.clearDisplay();
    // display.fillRect(0, 28, 128, 46, SSD1306_BLACK);
    display.setTextSize(2);               // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);  // Draw white text
    display.setCursor(0, 0);              // Start at top-left corner
    /////////////////////////////////////////////////////////////////////////////////
    // level_read();
    display.clearDisplay();
    display.setTextSize(3);

    if (level >= 0) {
      display.setCursor(40, 10);
      display.print(level);  //////////////////////////////////////
    } else {
      display.setCursor(50, 10);
      display.print("0");
    }
    display.setTextSize(3);
    //display.setCursor(70, 20);
    display.print("%");
    if (digitalRead(genarator_status) == LOW) {
      display.setCursor(35, 50);
      display.setTextSize(2);
      display.print("GN ON");
    } else {
      display.setCursor(35, 50);
      display.setTextSize(2);
      display.print("GN OFF");
    }
    display.display();
    Scheduler.delay(10000);
    manu = 0;
    ///////////////////////////////////////////////////////////////////////
  }
  if (manu == 0) {
    display.clearDisplay();
    display.fillRect(0, 28, 128, 46, SSD1306_BLACK);
    display.setCursor(25, 1);
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.println(DayMonthYear(dt.day, dt.month, dt.year));
    display.setCursor(45, 15);
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.println(DayOfTheWeek(dt.dayOfWeek));
    display.setCursor(20, 40);
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    display.println(CurrentTime(dt.hour, dt.minute));
    display.display();
  }
  if (level == fuel_level) {
    fl = 1;
    flr = level + 3;
    if (one_time_sms == 0) {
      one_time_sms = 1;
      send_sms();
    }
  }
  if (level >= flr) {
    // fuel_level = fuel_level - 2;
    fl = 0;
    one_time_sms = 0;
  }
  level_read();
  Scheduler.delay(20);
}
void level_read() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distance = (duration * 0.034 / 2) - 3;
  level = (distance / height) * 100;
  level = 100 - level;
}
