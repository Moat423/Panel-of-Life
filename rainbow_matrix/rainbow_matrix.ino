#include <FastLED.h>

// How many leds in your strip?
#define NUM_LEDS 256 

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 13
//#define CLOCK_PIN 13
#define MATRIX_WIDTH  16
#define MATRIX_HEIGHT 16 

// Define the array of leds
CRGB leds[NUM_LEDS];

void setup() { 
  Serial.begin(57600);
  Serial.println("resetting");
  LEDS.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
  LEDS.setBrightness(84);
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

// Map (row, col)
uint16_t XY(uint8_t x, uint8_t y) {
  if (y % 2 == 0) {
    // Even rows left to right
    return (y * MATRIX_WIDTH) + x;
  } else {
    // Odd rows right to left
    return (y * MATRIX_WIDTH) + (MATRIX_WIDTH - 1 - x);
  }
}

void loop() { 
  static uint8_t hue = 0;
  Serial.print("x");
  // First slide the led in one direction
  for(uint8_t i = 0; i < MATRIX_HEIGHT; i++) {
    for (uint8_t j = 0; j < MATRIX_WIDTH; j++) {
      uint16_t idx = XY(j, i);
      // Set the idx'th led to red 
      leds[idx] = CHSV(hue++, 255, 255);
      // Show the leds
      FastLED.show(); 
      // now that we've shown the leds, reset the i'th led to black
      // leds[i] = CRGB::Black;
      fadeall();
      // Wait a little bit before we loop around and do it again
      delay(10);
     }
   }

  Serial.print("x");
 }
