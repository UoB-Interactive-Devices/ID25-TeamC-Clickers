#include "OLED_Driver.h"
#include "GUI_paint.h"
#include "DEV_Config.h"
#include "Debug.h"
#include "ImageData.h"
#include "HX711.h"

#define CLK  A0
#define DOUT A1
#define INP  9
#define CALIBRATION_FACTOR 0.51

HX711 scale;
float weight = 0.00;
UBYTE *BlackImage;

void setup() {
    System_Init();
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

    Paint_DrawString_EN(10, 10, "Weight:", &Font16, WHITE, BLACK);
    Paint_DrawString_EN(110, 30, "g", &Font20, WHITE, BLACK);
    
    OLED_0in96_display(BlackImage); // Display initial screen

    pinMode(INP, INPUT);
    
    scale.begin(DOUT, CLK);
    scale.set_scale(CALIBRATION_FACTOR);
    scale.tare(); // Reset scale to zero
}

void loop() {
    if(digitalRead(INP) == HIGH){
      Serial.println("button pressed");
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

    Serial.print("Weight: ");
    Serial.print(weight, 2);
    Serial.println(" g");

    scale.power_down();	
    delay(500); // Small delay to reduce flickering
    scale.power_up();
}