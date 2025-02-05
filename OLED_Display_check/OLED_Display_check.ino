#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED display I2C address is usually 0x3C or 0x3D
#define OLED_ADDRESS 0x3C  

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing OLED...");

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("SSD1306 allocation failed. Check wiring!");
    while (1); // Stop if display is not found
  }

  Serial.println("OLED Initialized Successfully!");

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println("Hello, World!");
  display.display();

  delay(2000);  // Pause for 2 seconds to display message
}

void loop() {
  // You can add more display updates here
}
