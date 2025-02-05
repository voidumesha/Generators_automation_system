// Digital Output Pins
#define FLAME_D1 30
#define FLAME_D2 31
#define FLAME_D3 32
#define FLAME_D4 33
#define FLAME_D5 34

// Analog Output Pins
#define FLAME_A1 A0
#define FLAME_A2 A1
#define FLAME_A3 A2
#define FLAME_A4 A3
#define FLAME_A5 A4

void setup() {
  Serial.begin(9600);

  pinMode(FLAME_D1, INPUT);
  pinMode(FLAME_D2, INPUT);
  pinMode(FLAME_D3, INPUT);
  pinMode(FLAME_D4, INPUT);
  pinMode(FLAME_D5, INPUT);

  pinMode(FLAME_A1, INPUT);
  pinMode(FLAME_A2, INPUT);
  pinMode(FLAME_A3, INPUT);
  pinMode(FLAME_A4, INPUT);
  pinMode(FLAME_A5, INPUT);
}

void loop() {
  // Read digital values (fire detected if LOW)
  int flameD1 = digitalRead(FLAME_D1);
  int flameD2 = digitalRead(FLAME_D2);
  int flameD3 = digitalRead(FLAME_D3);
  int flameD4 = digitalRead(FLAME_D4);
  int flameD5 = digitalRead(FLAME_D5);

  // Read analog values (0-1023)
  int flameA1 = analogRead(FLAME_A1);
  int flameA2 = analogRead(FLAME_A2);
  int flameA3 = analogRead(FLAME_A3);
  int flameA4 = analogRead(FLAME_A4);
  int flameA5 = analogRead(FLAME_A5);

  Serial.println("=== Flame Sensor Readings ===");

  Serial.print("D1: "); Serial.print(flameD1 == LOW ? "🔥 Fire! " : "✅ Safe ");
  Serial.print(" A1: "); Serial.println(flameA1);

  Serial.print("D2: "); Serial.print(flameD2 == LOW ? "🔥 Fire! " : "✅ Safe ");
  Serial.print(" A2: "); Serial.println(flameA2);

  Serial.print("D3: "); Serial.print(flameD3 == LOW ? "🔥 Fire! " : "✅ Safe ");
  Serial.print(" A3: "); Serial.println(flameA3);

  Serial.print("D4: "); Serial.print(flameD4 == LOW ? "🔥 Fire! " : "✅ Safe ");
  Serial.print(" A4: "); Serial.println(flameA4);

  Serial.print("D5: "); Serial.print(flameD5 == LOW ? "🔥 Fire! " : "✅ Safe ");
  Serial.print(" A5: "); Serial.println(flameA5);

  Serial.println("=============================");
  delay(1000);
}
