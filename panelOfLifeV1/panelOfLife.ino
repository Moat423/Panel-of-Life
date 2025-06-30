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

// Action Button Configuration
#define ACTION_BUTTON_PIN 2  // Action button on pin 2

// Default values when axis not actioned
#define ANALOG_X_CORRECTION 122
#define ANALOG_Y_CORRECTION 129

// Define the array of leds
CRGB leds[NUM_LEDS];

// Keep track of drawn pixels - will be used for Game of Life
bool drawnPixels[NUM_LEDS] = {false};
bool nextGeneration[NUM_LEDS] = {false}; // Buffer for next generation

// Cursor position
uint8_t cursorX = MATRIX_WIDTH / 2;  // Start in the middle
uint8_t cursorY = MATRIX_HEIGHT / 2; // Start in the middle

// Cursor and drawing colors
CRGB cursorColor = CRGB::Green;
CRGB drawColor = CRGB::Red;
CRGB lifeColor = CRGB::Red;  // Same color for Game of Life cells

// Operating modes
#define MODE_DRAWING 0
#define MODE_GAME_OF_LIFE 1
uint8_t currentMode = MODE_DRAWING;

// Button timing variables
unsigned long lastActionButtonPress = 0;
bool actionButtonPressed = false;
bool actionButtonWasPressed = false;

struct analog {
  short x, y;
  bool buttonPressed;
};

// Game of Life generation counter
unsigned int generationCount = 0;
unsigned long lastGenerationTime = 0;
#define GENERATION_DELAY 500  // Time between generations in milliseconds

void setup() {
  // Initialize joystick
  pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize action button
  pinMode(ACTION_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize LED matrix
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS); // GRB color order for WS2812
  FastLED.setBrightness(84);
  
  // Clear the display
  FastLED.clear();
  FastLED.show();
  
  Serial.begin(115200);
  Serial.println("Joystick LED Drawing Tool with Game of Life initialized");
}

void loop() {
  // Read joystick values
  analog joystick;
  joystick.x = readAnalogAxisLevel(ANALOG_X_PIN) - ANALOG_X_CORRECTION;
  joystick.y = readAnalogAxisLevel(ANALOG_Y_PIN) - ANALOG_Y_CORRECTION;
  joystick.buttonPressed = isAnalogButtonPressed(ANALOG_BUTTON_PIN);
  
  // Handle the action button with improved logic
  handleActionButton();
  
  // Handle the current mode
  if (currentMode == MODE_DRAWING) {
    handleDrawingMode(joystick);
  } else if (currentMode == MODE_GAME_OF_LIFE) {
    handleGameOfLifeMode();
  }
  
  delay(50); // Small delay for smoother operation
}

void handleActionButton() {
  // Read current state of action button (LOW when pressed because of INPUT_PULLUP)
  bool currentButtonState = (digitalRead(ACTION_BUTTON_PIN) == LOW);
  
  // Button press detection (edge detection - only triggers once when button is pressed)
  if (currentButtonState && !actionButtonPressed) {
    actionButtonPressed = true;
    
    // Check for double press (two presses within 500ms)
    if (millis() - lastActionButtonPress < 500) {
      // Double press detected - clear the grid
      clearAllDrawnPixels();
      currentMode = MODE_DRAWING;
      Serial.println("Double press - Clearing grid and returning to drawing mode");
    } else {
      // Single press - toggle mode
      if (currentMode == MODE_DRAWING) {
        currentMode = MODE_GAME_OF_LIFE;
        generationCount = 0;
        lastGenerationTime = millis();
        Serial.println("Switching to Game of Life mode");
      } else {
        currentMode = MODE_DRAWING;
        Serial.println("Switching to Drawing mode");
      }
    }
    
    lastActionButtonPress = millis();
  }
  // Button release detection
  else if (!currentButtonState && actionButtonPressed) {
    actionButtonPressed = false;
  }
  
  // Update previous button state
  actionButtonWasPressed = actionButtonPressed;
}

void handleDrawingMode(analog joystick) {
  // Store previous cursor position
  uint16_t oldCursorIndex = XY(cursorX, cursorY);
  
  // Update cursor position based on joystick input
  updateCursorPosition(joystick.x, joystick.y);
  
  // Get new cursor position
  uint16_t newCursorIndex = XY(cursorX, cursorY);
  
  // Check if joystick button is pressed to draw at current position
  if (joystick.buttonPressed) {
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
  Serial.print("Drawing Mode | X: ");
  Serial.print(cursorX);
  Serial.print(" | Y: ");
  Serial.print(cursorY);
  Serial.print(" | Button: ");
  Serial.println(joystick.buttonPressed ? "Pressed" : "Not pressed");
  delay(170);
}

void handleGameOfLifeMode() {
  unsigned long currentTime = millis();
  
  // Only update at specific intervals
  if ((currentTime - lastGenerationTime) >= GENERATION_DELAY) {
    // Time to calculate next generation
    evolveGameOfLife();
    
    // Update display to show current generation
    updateGameOfLifeDisplay();
    
    // Update timing variables
    lastGenerationTime = currentTime;
    generationCount++;
    
    Serial.print("Game of Life Mode | Generation: ");
    Serial.println(generationCount);
  }
}

void evolveGameOfLife() {
  // Calculate next generation based on current state
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      int currentIndex = XY(x, y);
      bool currentState = drawnPixels[currentIndex];
      int neighbors = countLiveNeighbors(x, y);
      
      // Apply Game of Life rules
      if (currentState) { // Cell is alive
        // Live cell with 2 or 3 live neighbors survives
        nextGeneration[currentIndex] = (neighbors == 2 || neighbors == 3);
      } else { // Cell is dead
        // Dead cell with exactly 3 live neighbors becomes alive
        nextGeneration[currentIndex] = (neighbors == 3);
      }
    }
  }
  
  // Copy next generation to current
  for (int i = 0; i < NUM_LEDS; i++) {
    drawnPixels[i] = nextGeneration[i];
  }
}

int countLiveNeighbors(int x, int y) {
  int count = 0;
  
  // Check all 8 surrounding cells
  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      // Skip the cell itself
      if (dx == 0 && dy == 0) {
        continue;
      }
      
      int nx = x + dx;
      int ny = y + dy;
      
      // Make sure we don't go out of bounds (no wrap-around)
      if (nx >= 0 && nx < MATRIX_WIDTH && ny >= 0 && ny < MATRIX_HEIGHT) {
        // Check if this neighbor is alive
        if (drawnPixels[XY(nx, ny)]) {
          count++;
        }
      }
    }
  }
  
  return count;
}

void updateGameOfLifeDisplay() {
  // Update the LED display based on the current state
  FastLED.clear();
  
  // Display all live cells
  for (int i = 0; i < NUM_LEDS; i++) {
    if (drawnPixels[i]) {
      leds[i] = lifeColor;
    }
  }
  
  // Show the updated display
  FastLED.show();
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
  memset(nextGeneration, 0, sizeof(nextGeneration));
  
  // Clear the display
  FastLED.clear();
  
  // Set only the cursor position if in drawing mode
  if (currentMode == MODE_DRAWING) {
    uint16_t cursorIndex = XY(cursorX, cursorY);
    leds[cursorIndex] = cursorColor;
  }
  
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
