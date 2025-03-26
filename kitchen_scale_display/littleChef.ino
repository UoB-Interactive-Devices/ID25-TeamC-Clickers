#include <Servo.h>
#include "OLED_Driver.h"
#include "GUI_paint.h"
#include "DEV_Config.h"
#include "Debug.h"
#include "ImageData.h"
#include "HX711.h"

// Servo Setup
Servo shoulderServo;
int servoPin = 6;
int minAngle = 0;
int maxAngle = 50;
int stepDelay = 20;

// Touch Sensor & Serial Communication
#define TOUCH_SENSOR 12  

// Scale Setup
#define CLK A0
#define DOUT A1
#define INP 9  // Scale reset button
#define CALIBRATION_FACTOR 0.51

HX711 scale;
float weight = 0.00;
UBYTE *BlackImage;

void setup() {
    Serial.begin(9600);
    
    // Initialize OLED Display
    System_Init();
    OLED_0in96_Init();
    OLED_0in96_clear();
    UWORD ImageSize = ((OLED_0in96_WIDTH % 8 == 0) ? (OLED_0in96_WIDTH / 8) : (OLED_0in96_WIDTH / 8 + 1)) * OLED_0in96_HEIGHT;
    
    if ((BlackImage = (UBYTE *)malloc(ImageSize)) == NULL) { 
        Serial.println("Failed to allocate memory!");
        while (1);
    }
    
    Paint_NewImage(BlackImage, OLED_0in96_WIDTH, OLED_0in96_HEIGHT, 90, BLACK);
    Paint_SelectImage(BlackImage);
    Paint_Clear(BLACK);
    Paint_DrawString_EN(10, 10, "Weight:", &Font16, WHITE, BLACK);
    Paint_DrawString_EN(110, 30, "g", &Font20, WHITE, BLACK);
    OLED_0in96_display(BlackImage);

    // Initialize Scale
    pinMode(INP, INPUT);
    scale.begin(DOUT, CLK);
    scale.set_scale(CALIBRATION_FACTOR);
    scale.tare(); // Reset scale to zero

    // Initialize Servo & Touch Sensor
    shoulderServo.attach(servoPin);
    pinMode(TOUCH_SENSOR, INPUT);
}

void loop() {
    handleScale();  // Keeps updating weight
    handleTouchSensor();  // Sends "START" when pressed
    handleSerialInput();  // Listens for "TIMER_DONE" from laptop
}

// Function to handle scale operations
void handleScale() {
    if (digitalRead(INP) == HIGH) {
        Serial.println("Button pressed, resetting scale...");
        scale.tare();
    }

    weight = scale.get_units(12);
    char weightStr[10];
    dtostrf(weight, 6, 2, weightStr);

    Paint_Clear(BLACK);
    Paint_DrawString_EN(10, 10, "Weight:", &Font16, WHITE, WHITE);
    Paint_DrawNum(5, 30, weightStr, &Font20, 2, WHITE, WHITE);
    Paint_DrawString_EN(110, 30, "g", &Font20, WHITE, WHITE);
    OLED_0in96_display(BlackImage);

    Serial.print("WEIGHT:");
    Serial.println(weight, 2);

    scale.power_down();
    delay(500);
    scale.power_up();
}

// Function to handle touch sensor input
void handleTouchSensor() {
    if (digitalRead(TOUCH_SENSOR) == HIGH) {
        Serial.println("START");  // Send signal to laptop
        delay(500);  // Debounce delay
    }
}

// Function to listen for messages from the laptop
void handleSerialInput() {
    if (Serial.available()) {
        String message = Serial.readStringUntil('\n');
        Serial.println(message);
        
        if (message == "TIMER_DONE") {
            moveServo();  
        }
    }
}

// Function to move servo
void moveServo() {
    for (int pos = minAngle; pos <= maxAngle; pos++) {
        shoulderServo.write(pos);
        delay(stepDelay);
    }
    for (int pos = maxAngle; pos >= minAngle; pos--) {
        shoulderServo.write(pos);
        delay(stepDelay);
    }
}
