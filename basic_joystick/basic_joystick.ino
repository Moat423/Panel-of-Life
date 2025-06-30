#define ANALOG_X_PIN A2 
#define ANALOG_Y_PIN A3 
#define ANALOG_BUTTON_PIN A4 
   
//Default values when axis not actioned 
#define ANALOG_X_CORRECTION 122 
#define ANALOG_Y_CORRECTION 129 
   
struct button { 
   byte pressed = 0; 
}; 
   
struct analog { 
   short x, y; 
   
   button button; 
}; 
   
void setup() 
{ 
   pinMode(ANALOG_BUTTON_PIN, INPUT_PULLUP); 
   
   Serial.begin(115200); 
} 
   
void loop() 
{ 
   analog analog; 
   
   analog.x = readAnalogAxisLevel(ANALOG_X_PIN) - ANALOG_X_CORRECTION; 
   analog.y = readAnalogAxisLevel(ANALOG_Y_PIN) - ANALOG_Y_CORRECTION; 
   
   analog.button.pressed = isAnalogButtonPressed(ANALOG_BUTTON_PIN); 
   
   Serial.print("X:"); 
   Serial.println(analog.x); 
   
   Serial.print("Y:"); 
   Serial.println(analog.y); 
   
   if (analog.button.pressed) { 
     Serial.println("Button pressed"); 
   } else { 
     Serial.println("Button not pressed"); 
   } 
   
   delay(200); 
} 
   
byte readAnalogAxisLevel(int pin) 
{ 
   return map(analogRead(pin), 0, 1023, 0, 255); 
} 
   
bool isAnalogButtonPressed(int pin) 
{ 
   return digitalRead(pin) == 0; 
} 
