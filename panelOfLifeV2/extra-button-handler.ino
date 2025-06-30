#include <FastLED.h>

#define BRIGHTNESS 30

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

// Timing configurations
#define CURSOR_MOVE_DELAY 170    // Delay between cursor movements (ms)
#define GENERATION_DELAY 500     // Time between Game of Life generations (ms)
#define DOUBLE_PRESS_WINDOW 500  // Window for detecting double press (ms)

// Operating modes
#define MODE_DRAWING 0
#define MODE_GAME_OF_LIFE 1

// Define the array of leds
CRGB leds[NUM_LEDS];

// Keep track of drawn pixels for Game of Life
bool drawnPixels[NUM_LEDS] = {false};
bool nextGeneration[NUM_LEDS] = {false}; // Buffer for next generation

// Cursor position
uint8_t cursorX = MATRIX_WIDTH / 2;  // Start in the middle
uint8_t cursorY = MATRIX_HEIGHT / 2; // Start in the middle

// Colors
CRGB cursorColor = CRGB::Green;
CRGB drawColor = CRGB(180, 10, 220);
CRGB lifeColor = CRGB(255, 240, 0);

// State variables
uint8_t currentMode = MODE_DRAWING;
unsigned long lastCursorMoveTime = 0;
unsigned long lastGenerationTime = 0;
unsigned int generationCount = 0;

// Button timing variables
unsigned long lastActionButtonPress = 0;
bool actionButtonPressed = false;

void setup() {
  // Initialize pins
  pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ACTION_BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize LED matrix
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(BRIGHTNESS);
  
  // Clear the display
  FastLED.clear();
  FastLED.show();
  
  Serial.begin(115200);
  Serial.println("Joystick LED Drawing Tool with Game of Life initialized");
}

void loop() {
  // Handle action button (with double-press detection)
  handleActionButton();
  
  // Handle the current mode
  if (currentMode == MODE_DRAWING) {
    handleDrawingMode();
  } else { // MODE_GAME_OF_LIFE
    handleGameOfLifeMode();
  }
  
  delay(20); // Small delay for system responsiveness
}

void handleActionButton() {
  // Read current button state (LOW when pressed because of INPUT_PULLUP)
  bool currentButtonState = (digitalRead(ACTION_BUTTON_PIN) == LOW);
  
  // Button press detection (edge detection)
  if (currentButtonState && !actionButtonPressed) {
    actionButtonPressed = true;
    
    // Check for double press (two presses within DOUBLE_PRESS_WINDOW)
    if (millis() - lastActionButtonPress < DOUBLE_PRESS_WINDOW) {
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
}

void handleDrawingMode() {
  // Get current time for cursor movement timing
  unsigned long currentTime = millis();
  
  short joystickX = readAnalogAxisLevel(ANALOG_X_PIN) - ANALOG_X_CORRECTION;
  short joystickY = readAnalogAxisLevel(ANALOG_Y_PIN) - ANALOG_Y_CORRECTION;
  bool joystickButtonPressed = isAnalogButtonPressed(ANALOG_BUTTON_PIN);

  uint16_t oldCursorIndex = XY(cursorX, cursorY);
  
  // Only update cursor position at controlled intervals
  if (currentTime - lastCursorMoveTime >= CURSOR_MOVE_DELAY) {
    updateCursorPosition(joystickX, joystickY);
    lastCursorMoveTime = currentTime;
  }
  uint16_t newCursorIndex = XY(cursorX, cursorY);
  
  if (joystickButtonPressed) {
    drawnPixels[newCursorIndex] = true;
  }
  
  // Only update LEDs that need changing
  if (oldCursorIndex != newCursorIndex) {
    if (drawnPixels[oldCursorIndex]) {
      leds[oldCursorIndex] = drawColor;
    } else {
      leds[oldCursorIndex] = CRGB::Black;
    }
  }
  
  leds[newCursorIndex] = cursorColor;

  FastLED.show();
  
  Serial.print("Drawing Mode | X: ");
  Serial.print(cursorX);
  Serial.print(" | Y: ");
  Serial.print(cursorY);
  Serial.print(" | Button: ");
  Serial.println(joystickButtonPressed ? "Pressed" : "Not pressed");
}

void handleGameOfLifeMode() {
  unsigned long currentTime = millis();
  
  if ((currentTime - lastGenerationTime) >= GENERATION_DELAY) {
    evolveGameOfLife();
    updateGameOfLifeDisplay();
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
      
      if (currentState) { 
        nextGeneration[currentIndex] = (neighbors == 2 || neighbors == 3);
      } else { 
        nextGeneration[currentIndex] = (neighbors == 3);
      }
    }
  }
  memcpy(drawnPixels, nextGeneration, sizeof(drawnPixels));
}

int countLiveNeighbors(int x, int y) {
  int count = 0;

  for (int dy = -1; dy <= 1; dy++) {
    for (int dx = -1; dx <= 1; dx++) {
      if (dx == 0 && dy == 0) continue;
      
      int nx = x + dx;
      int ny = y + dy;
      
      if (nx >= 0 && nx < MATRIX_WIDTH && ny >= 0 && ny < MATRIX_HEIGHT) {
        if (drawnPixels[XY(nx, ny)]) count++;
      }
    }
  }
  return count;
}

void updateGameOfLifeDisplay() {
  FastLED.clear();
  
  // Display all live cells
  for (int i = 0; i < NUM_LEDS; i++) {
    if (drawnPixels[i]) {
      leds[i] = lifeColor;
    }
  }
  FastLED.show();
}

// Map from (x, y) coordinates to LED index, accounting for zigzag pattern
uint16_t XY(uint8_t x, uint8_t y) {
  if (y % 2 == 0) {
    // Even rows left to right
    return (y * MATRIX_WIDTH) + x;
  } else {
    return (y * MATRIX_WIDTH) + (MATRIX_WIDTH - 1 - x);
  }
}

void updateCursorPosition(short joystickX, short joystickY) {
  if (joystickX > 20) {
    cursorX = min(cursorX + 1, MATRIX_WIDTH - 1);
  } else if (joystickX < -20) {
    cursorX = max(cursorX - 1, 0);
  }

  if (joystickY < -20) {
    cursorY = min(cursorY + 1, MATRIX_HEIGHT - 1);
  } else if (joystickY > 20) {
    cursorY = max(cursorY - 1, 0);
  }
}

// Clear all drawn pixels
void clearAllDrawnPixels() {
  memset(drawnPixels, 0, sizeof(drawnPixels));
  memset(nextGeneration, 0, sizeof(nextGeneration));
  FastLED.clear();
  if (currentMode == MODE_DRAWING) {
    uint16_t cursorIndex = XY(cursorX, cursorY);
    leds[cursorIndex] = cursorColor;
  }
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
