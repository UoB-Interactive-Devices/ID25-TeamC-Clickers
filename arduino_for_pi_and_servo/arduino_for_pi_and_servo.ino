#include <Servo.h>

const int buttonPin = 2;   // state change button connects to arduino D2
const int servoPin = 9;    // servo connect to D9
int imageIndex = 1;        // remember the index 1, 2, 3 = the photo 1,2,3
bool lastButtonState = HIGH;
Servo myServo;

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.begin(9600);
  myServo.attach(servoPin);
  updateServo();  // init the servo status
}

void loop() {
  bool buttonState = digitalRead(buttonPin); //read the button status now

  // the button state changes when press the button
  if (buttonState == LOW && lastButtonState == HIGH) {
    imageIndex = (imageIndex % 3) + 1;  // Cycling Switch image 1-2-3-1-2-...
    Serial.print("STATE ");
    Serial.println(imageIndex);  // this two line: send "state 1/2/3" to port
    updateServo(); //change servo mode based on image index
    delay(300);  // avoid accidental touch within short time
  }

  lastButtonState = buttonState; //update button state

  if (imageIndex == 3) {
    swingServo(); //on state 3 is swing the servo(waving the arm))
  }
}

// update servo status
void updateServo() {
  if (imageIndex == 1) {
    myServo.write(0);  // image 1 = 0 deg
  } else if (imageIndex == 2) {
    myServo.write(90);  // image 2 = 90 deg
  } else {
    myServo.write(0);  // image 3 = swing
  }
}

// swing between -10 and +10 deg
void swingServo() {
  static int angle = -10;
  static bool increasing = true;

  if (increasing) {
    angle += 5;  //swing faster
    if (angle >= 10) increasing = false; //change direction when upto +10 deg
  } else {
    angle -= 5;
    if (angle <= -10) increasing = true; //change direction when lowerto -10 deg
  }

  myServo.write(angle + 90); //actual position = change angle + 90 deg
  delay(30);  
}
