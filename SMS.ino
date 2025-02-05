
void SMS(void) {
  if (one_time == 0) {
    Serial1.println("AT");
    Scheduler.delay(1000);
    // Enable verbose responses for better debugging
    Serial1.println("AT+CMGF=1");
    Scheduler.delay(1000);
    // Display SMS messages in text mode
    Serial1.println("AT+CNMI=1,2,0,0,0");
    Scheduler.delay(1000);
    one_time = 1;
  }
  if (Serial1.available() > 0) {
    textMessage = Serial1.readString();
    textMessage.toUpperCase();
    CellNumtemp = textMessage.substring(textMessage.indexOf("+94"));
    CellNum = CellNumtemp.substring(0, 12);
    //   String c = response.substring(1);
    Scheduler.delay(10);
    if (textMessage.indexOf("FLEVEL") >= 0) {
      manu = 3;
      display.clearDisplay();
      display.setCursor(25, 20);
      display.setTextSize(2);
      display.print("Sending");
      display.setCursor(50, 40);
      display.setTextSize(2);
      display.print("SMS");
      display.display();

      Serial1.println("AT");  // Handshake test, should return "OK" on success
      Scheduler.delay(100);
      Serial1.println("AT+CMGF=1");  // Configuring TEXT mode
      Scheduler.delay(100);
      Serial1.print("AT+CMGS=\"");
      Serial1.print(CellNum);
      Serial1.println("\"");
      Scheduler.delay(100);
      if (fl == 1) {
        Serial1.print("Low fuel level ");
      } else {
        Serial1.print("Fuel level ");
      }
      Serial1.print(level);
      Serial1.print("%");
      if (digitalRead(genarator_status) == LOW) {
        Serial1.println(" , Genarator running");
      } else {
        Serial1.println(" , Genarator turn OFF");
      }
      Scheduler.delay(100);
      Serial1.write(26);
      Scheduler.delay(3000);
      manu = 0;
      one_time = 0;
      textMessage = "";
      CellNum = "";
      CellNumtemp = "";
    }
  }
  Scheduler.delay(1);
}
void send_sms() {
  display.clearDisplay();
  display.setCursor(25, 20);
  display.setTextSize(2);
  display.print("Sending");
  display.setCursor(50, 40);
  display.setTextSize(2);
  display.print("SMS");
  display.display();

  Serial1.println("AT");  // Handshake test, should return "OK" on success
  Scheduler.delay(100);
  Serial1.println("AT+CMGF=1");  // Configuring TEXT mode
  Scheduler.delay(100);
  Serial1.print("AT+CMGS=\"");
  Serial1.print(phoneNumber2);
  Serial1.println("\"");
  Scheduler.delay(100);
  if (fl == 1) {
    Serial1.print("Low fuel level ");
  } else {
    Serial1.print("Fuel level ");
  }
  Serial1.print(level);
  Serial1.print("%");
  if (digitalRead(genarator_status) == LOW) {
    Serial1.println(" , Genarator running");
  } else {
    Serial1.println(" , Genarator turn OFF");
  }
  Scheduler.delay(100);
  Serial1.write(26);
  Scheduler.delay(5000);
  ///////////////////////////////////////////////////////////////////////////////////
  Serial1.println("AT");  // Handshake test, should return "OK" on success
  Scheduler.delay(100);
  Serial1.println("AT+CMGF=1");  // Configuring TEXT mode
  Scheduler.delay(100);
  Serial1.print("AT+CMGS=\"");
  Serial1.print(phoneNumber);
  Serial1.println("\"");
  Scheduler.delay(100);
  if (fl == 1) {
    Serial1.print("Low fuel level ");
   // one_time_sms = 1;
  } else {
    Serial1.print("Fuel level ");
  }
  Serial1.print(level);
  Serial1.print("%");
  if (digitalRead(genarator_status) == LOW) {
    Serial1.println(" , Genarator running");
  } else {
    Serial1.println(" , Genarator turn OFF");
  }
  Scheduler.delay(100);
  Serial1.write(26);
  Scheduler.delay(3000);
  manu = 0;
  one_time = 0;
}