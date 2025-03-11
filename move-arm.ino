#include <Servo.h>

Servo shoulderServo;  
// Create a Servo object
int servoPin = 9;     // Pin the servo is attached to
int minAngle = 0;     // Min angle (servo position)
int maxAngle = 50;    // Max angle (servo position)
int stepDelay = 20;     // 

void setup() {
    Serial.begin(9600);  
    shoulderServo.attach(servoPin);  
}

void loop() {
    if (Serial.available()) {
        String message = Serial.readStringUntil('\n');  //read data from computer
         Serial.println(message);
        
        if (message == "TIMER_DONE") {
            moveServo();  // move when gets signal
        }
    }
}

// move arm back and forth
void moveServo() {
    for (int pos = minAngle; pos <= maxAngle; pos++) {
        shoulderServo.write(pos);
        delay(stepDelay);
    }
    for (int pos = maxAngle; pos >= minAngle; pos--) {
        shoulderServo.write(pos);
        delay(stepDelay);
    }

    for (int pos = minAngle; pos <= maxAngle; pos++) {
        shoulderServo.write(pos);
        delay(stepDelay);
    }
    for (int pos = maxAngle; pos >= minAngle; pos--) {
        shoulderServo.write(pos);
        delay(stepDelay);
    }
}
