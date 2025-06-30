#include <FastLED.h>

// LED Matrix Configuration
#define NUM_LEDS 256
#define DATA_PIN 13
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16

// Joystick Configuration
#define ANALOG_X_PIN A2
#define ANALOG_Y_PIN A3
#define ANALOG_BUTTON_PIN A4

// Reset Button Configuration
#define RESET_BUTTON_PIN 2  // Reset button on pin 2

// Default values when axis not actioned
#define ANALOG_X_CORRECTION 122
#define ANALOG_Y_CORRECTION 129

// Define the array of leds
CRGB leds[NUM_LEDS];

// Keep track of drawn pixels - will be used for Game of Life later
bool drawnPixels[NUM_LEDS] = {false};

// Cursor position
uint8_t cursorX = MATRIX_WIDTH / 2;  // Start in the middle
uint8_t cursorY = MATRIX_HEIGHT / 2; // Start in the middle
uint16_t prevCursorIndex = 0; // Track previous cursor position

// Cursor and drawing colors
CRGB cursorColor = CRGB::Green;
CRGB drawColor = CRGB::Red;

// Button structure
struct button {
  byte pressed = 0;
};

// Reset button structure
struct resetButton {
  byte pressed = 0;
  byte prevPressed = 0;
};

struct analog {
  short x, y;
  button button;
};

// Global reset button state
resetButton resetBtn;

void setup() {
  // Initialize joystick
  pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize reset button
  pinMode(RESET_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize LED matrix
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS); // GRB color order for WS2812
  FastLED.setBrightness(84);
  
  // Clear the display
  FastLED.clear();
  FastLED.show();
  
  Serial.begin(115200);
  Serial.println("Joystick LED Drawing Tool initialized");
  
  // Initialize previous cursor index
  prevCursorIndex = XY(cursorX, cursorY);
}

void loop() {
  // Read joystick values
  analog joystick;
  joystick.x = readAnalogAxisLevel(ANALOG_X_PIN) - ANALOG_X_CORRECTION;
  joystick.y = readAnalogAxisLevel(ANALOG_Y_PIN) - ANALOG_Y_CORRECTION;
  
  // Get current joystick button state
  joystick.button.pressed = isAnalogButtonPressed(ANALOG_BUTTON_PIN);
  
  // Get current reset button state (LOW when pressed because of INPUT_PULLUP)
  resetBtn.pressed = (digitalRead(RESET_BUTTON_PIN) == LOW);
  
  // Check for reset button press (edge detection)
  if (resetBtn.pressed && !resetBtn.prevPressed) {
    clearAllDrawnPixels();
    Serial.println("Reset button pressed - Clearing screen");
  }
  
  // Store previous cursor position
  uint16_t oldCursorIndex = XY(cursorX, cursorY);
  
  // Update cursor position based on joystick input
  updateCursorPosition(joystick.x, joystick.y);
  
  // Get new cursor position
  uint16_t newCursorIndex = XY(cursorX, cursorY);
  
  // Check if joystick button is pressed to draw at current position
  if (joystick.button.pressed) {
    drawnPixels[newCursorIndex] = true;
  }
  
  // Only update LEDs that need changing
  
  // If cursor moved, update old position
  if (oldCursorIndex != newCursorIndex) {
    // If old position was drawn, set to draw color
    if (drawnPixels[oldCursorIndex]) {
      leds[oldCursorIndex] = drawColor;
    } else {
      // Otherwise turn it off
      leds[oldCursorIndex] = CRGB::Black;
    }
  }
  
  // Update new cursor position (always displays cursor color)
  leds[newCursorIndex] = cursorColor;
  
  // Only need to show the updated LEDs
  FastLED.show();
  
  // Debug output
  Serial.print("X: ");
  Serial.print(cursorX);
  Serial.print(" | Y: ");
  Serial.print(cursorY);
  Serial.print(" | Joystick Button: ");
  Serial.print(joystick.button.pressed ? "Pressed" : "Not pressed");
  Serial.print(" | Reset Button: ");
  Serial.println(resetBtn.pressed ? "Pressed" : "Not pressed");
  
  // Update previous button states for edge detection next loop
  resetBtn.prevPressed = resetBtn.pressed;
  
  delay(50); // Small delay for smoother movement
}

// Map from (x, y) coordinates to LED index, accounting for zigzag pattern
uint16_t XY(uint8_t x, uint8_t y) {
  if (y % 2 == 0) {
    // Even rows left to right
    return (y * MATRIX_WIDTH) + x;
  } else {
    // Odd rows right to left
    return (y * MATRIX_WIDTH) + (MATRIX_WIDTH - 1 - x);
  }
}

// Update cursor position based on joystick input
void updateCursorPosition(short joystickX, short joystickY) {
  // Only move if joystick is pushed significantly
  if (joystickX > 20) {
    cursorX = min(cursorX + 1, MATRIX_WIDTH - 1);
  } else if (joystickX < -20) {
    cursorX = max(cursorX - 1, 0);
  }
  
  // Y-axis inverted as requested
  if (joystickY < -20) {
    cursorY = min(cursorY + 1, MATRIX_HEIGHT - 1);
  } else if (joystickY > 20) {
    cursorY = max(cursorY - 1, 0);
  }
}

// Clear all drawn pixels
void clearAllDrawnPixels() {
  // Reset the drawn pixels array
  memset(drawnPixels, 0, sizeof(drawnPixels));
  
  // Clear the display
  FastLED.clear();
  
  // Set only the cursor position
  uint16_t cursorIndex = XY(cursorX, cursorY);
  leds[cursorIndex] = cursorColor;
  
  // Update the display
  FastLED.show();
}

// Read analog axis level from the joystick
byte readAnalogAxisLevel(int pin) {
  return map(analogRead(pin), 0, 1023, 0, 255);
}

// Check if the joystick button is pressed
bool isAnalogButtonPressed(int pin) {
  return digitalRead(pin) == 0;
}
