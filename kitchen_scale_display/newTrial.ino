#include "OLED_Driver.h"
#include "GUI_paint.h"
#include "DEV_Config.h"
#include "Debug.h"
#include "ImageData.h"
#include "HX711.h"
#include "Servo.h"

#define CLK  A0
#define DOUT A1
#define BUTTON_PIN 9
#define CALIBRATION_FACTOR 336.69
#define TOUCH_PIN 12
#define SERVO_PIN 6

Servo myServo;  // Create a Servo object
HX711 scale;
float weight = 0.00;
UBYTE *BlackImage;
volatile bool timerDoneFlag = false; // Flag to indicate if TIMER_DONE was received

void serialEvent() {
    String inputString = Serial.readStringUntil('\n');
    inputString.trim();
    if (inputString == "TIMER_DONE") {
        timerDoneFlag = true;
    }
}

void setup() {
    System_Init();
    Serial.println("Waiting for touch signal...");
    pinMode(TOUCH_PIN, INPUT);
    
    // Wait for touch signal
    while (digitalRead(TOUCH_PIN) == LOW);
    Serial.println("START");
    
    Serial.println(F("OLED_Init()..."));
    OLED_0in96_Init();
    OLED_0in96_clear();

    // Allocate memory for OLED image buffer
    UWORD ImageSize = ((OLED_0in96_WIDTH % 8 == 0) ? (OLED_0in96_WIDTH / 8) : (OLED_0in96_WIDTH / 8 + 1)) * OLED_0in96_HEIGHT;
    
    if ((BlackImage = (UBYTE *)malloc(ImageSize)) == NULL) { 
        Serial.println("Failed to allocate memory!");
        while (1); // Halt execution if memory allocation fails
    }
    
    Serial.println("Paint_NewImage");
    Paint_NewImage(BlackImage, OLED_0in96_WIDTH, OLED_0in96_HEIGHT, 90, BLACK);
    Paint_SelectImage(BlackImage);
    Paint_Clear(BLACK);
    Driver_Delay_ms(500);

    Paint_DrawString_EN(15, 10, "Weight:", &Font16, WHITE, WHITE);
    Paint_DrawNum(30, 30, "0.00", &Font20, 2, WHITE, WHITE);
    Paint_DrawString_EN(110, 30, "g", &Font20, WHITE, WHITE);
    
    OLED_0in96_display(BlackImage); // Display initial screen

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    myServo.attach(SERVO_PIN);
    myServo.write(0); // Set initial position
    
    scale.begin(DOUT, CLK);
    scale.set_scale(CALIBRATION_FACTOR);
    scale.tare(); // Reset scale to zero
}

void loop() {
    if (timerDoneFlag) {
        // Make the servo move up and down 3 times
        for (int i = 0; i < 3; i++) {
            // Move the servo to vertical position (90 degrees)
            myServo.write(90);
            Serial.println("Servo moving to 90 degrees");
            delay(100); // Wait for half a second

            // Move the servo to horizontal position (0 degrees)
            myServo.write(0);
            Serial.println("Servo moving to 0 degrees");
            delay(100); // Wait for half a second
        }

        // After the waving motion, reset the servo to horizontal position (0 degrees)
        myServo.write(0);
        Serial.println("Servo reset to 0 degrees");

        timerDoneFlag = false;
    }

    if (digitalRead(BUTTON_PIN) == 0) {
      Serial.println("button pressed");
      Paint_Clear(BLACK); // Clear previous display content
      Paint_DrawString_EN(40, 30, "TARE", &Font20, WHITE, WHITE);
      OLED_0in96_display(BlackImage);
      scale.tare();
    }

    weight = scale.get_units(12); // Get weight value
    char weightStr[10];
    dtostrf(weight, 6, 2, weightStr); // Convert float to string

    Paint_Clear(BLACK); // Clear previous display content
    Paint_DrawString_EN(10, 10, "Weight:", &Font16, WHITE, WHITE);
    Paint_DrawNum(5, 30, weightStr, &Font20, 2, WHITE, WHITE);
    Paint_DrawString_EN(110, 30, "g", &Font20, WHITE, WHITE);

    OLED_0in96_display(BlackImage);

    scale.power_down();    
    delay(500); // Small delay to reduce flickering
    scale.power_up();
}
