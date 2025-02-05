#include <SoftwareSerial.h>

#define SIM_TX 18
#define SIM_RX 19

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600); // Use Serial1 for Mega

  delay(1000);
  Serial.println("Sending SMS...");
  sendSMS("Adee cupiri ooiii");
}

void sendSMS(String message) {
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
  Serial.println("SMS Sent!");
}

void loop() {
}
