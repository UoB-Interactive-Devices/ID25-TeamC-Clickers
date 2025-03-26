#include <Servo.h>

#define ctsPin 11
Servo myServo;  // Create a Servo object

void setup() {
  pinMode(ctsPin, INPUT);
  myServo.attach(10); // Attach the servo to pin 9
  Serial.begin(9600);
}

void loop() {
  int ctsValue = digitalRead(ctsPin);

  if (ctsValue == HIGH) {
    myServo.write(90); // Move servo to 90 degrees
    Serial.println("TOUCHED");
  } else {
    myServo.write(0); // Move servo back to 0 degrees
    Serial.println("Not touched");
  }

  delay(500);
}
