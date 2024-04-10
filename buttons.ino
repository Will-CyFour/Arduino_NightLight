#include <math.h>

#define BUTTON 3

#define RED 11
#define GREEN 10
#define BLUE 9

#define LOFI_RESISTOR A0

#define PHOTO_RESISTOR A5



unsigned char mode = 0;
unsigned char color[3] = {200, 200, 0};

bool buttonPressed = false;

//Crossfade (mode 1)
float hue = 0.0; // Hue value (0 - 360)
float crossfade_vel = 2.0;

//Lemon Selector (mode 2)
const int colorMap[][3] = {
    {255, 0, 0},       // Red
    {255, 127, 0},     // Orange
    {255, 255, 0},     // Yellow
    {0, 255, 0},       // Green
    {0, 0, 255},       // Blue
    {75, 0, 130},      // Indigo
    {143, 0, 255}      // Violet
};

//Input (mode 3)
unsigned char inputIntegers[3]; // Array to hold the input integers
unsigned char index = 0; // Index to keep track of the current position in the inputIntegers array


void setup() {
  Serial.begin(9600);
  pinMode(BUTTON, INPUT_PULLUP);
  pinMode(RED, OUTPUT); 
  pinMode(BLUE, OUTPUT); 
  pinMode(GREEN, OUTPUT);

  delay(100);
  crossfadeSetup();
}

void loop() {
  delay((mode == 2) ? 1 : 100);

  //Check and handle button state
  if (!buttonPressed && isButtonPressed(BUTTON)){
    buttonPressed = true;
  }
  else if (buttonPressed && !isButtonPressed(BUTTON)){
    changeMode();
    buttonPressed = false;
  }

  //Mode desision tree
  switch (mode){
    case 0: crossfade(); break; //crossfade with light level
    case 1: lofi(); break; //Lo-fi analog input
    case 2: creative(); break; //Creative feature selecting color using Serial output from your computer
    default: return; break;
  }
}

void crossfade() {
  hue += crossfade_vel;
  float darkness = (analogRead(PHOTO_RESISTOR)) / 1000.0;
  setHSL(hue, 0.9, darkness); 
}

void crossfadeSetup() {
 hue = 0.0;
}

void lofi() {
  int lofiValue = analogRead(LOFI_RESISTOR);
  Serial.println(lofiValue);
  // Determine the index of the color in the rainbow based on analogValue
  unsigned char colorValue = map(lofiValue, 0, 1023, 0, 6);

  // Retrieve RGB values from the colorMap array
  unsigned char r = colorMap[colorValue][0];
  unsigned char g = colorMap[colorValue][1];
  unsigned char b = colorMap[colorValue][2];

  setRGB(r, g, b);
}

void lofiSetup() {

}

void creative() {
  if (Serial.available() > 0) { // Check if data is available to read
    char incomingChar = Serial.read(); // Read the incoming byte
    
    // If the incoming character is a digit
    if (isdigit(incomingChar)) {
      inputIntegers[index] = inputIntegers[index] * 10 + (incomingChar - '0'); // Convert character to integer and store in the current index of the array
    }
    // If the incoming character is a whitespace or a newline
    else if (incomingChar == ' ' || incomingChar == '\n' || incomingChar == ',') {
      char* rgb[] = {"Red: ", "Green: ", "Blue: "};
      Serial.print(rgb[index]);
      Serial.println(inputIntegers[index]);
      index++; // Move to the next index in the array
      
      // If we have read three integers, print them and reset the index and input array
      if (index == 3) {
        Serial.print("Setting color to: [");
        for (int i = 0; i < 3; i++) {
          Serial.print(inputIntegers[i]);
          if (i < 2) {
            Serial.print(", ");
          }
        }
        setRGB(inputIntegers[0], inputIntegers[1], inputIntegers[2]);//Set the light
        Serial.println("]");
        creativeSetup();
      }
    }
  }
}

void creativeSetup(){
    Serial.println("Input color in \"R G B\" format");
    index = 0; // Reset the index to start over
    memset(inputIntegers, 0, sizeof(inputIntegers)); // Reset the input array
}

void setRGB(unsigned char red, unsigned char green, unsigned char blue){
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  analogWrite(RED,color[0]);
  analogWrite(GREEN,color[1]);
  analogWrite(BLUE,color[2]);
}

void changeMode() {
  mode = (mode + 1) % 3;
  Serial.print("mode: ");
  Serial.println(mode + 1);
  switch (mode){
    case 0: crossfadeSetup(); break;
    case 1: lofiSetup(); break;
    case 2: setRGB(0,0,0); creativeSetup(); break;
  }
}

bool isButtonPressed(int buttonPin) {
  return digitalRead(buttonPin) == LOW;
}

void setHSL(float h, float s, float l) {
  float c = (1 - fabs(2 * l - 1)) * s;
  float x = c * (1 - fabs(fmod(h / 60.0, 2) - 1));
  float m = l - c / 2;
  
  float r_temp, g_temp, b_temp;

  if (h >= 0 && h < 60) {
    r_temp = c;
    g_temp = x;
    b_temp = 0;
  } else if (h >= 60 && h < 120) {
    r_temp = x;
    g_temp = c;
    b_temp = 0;
  } else if (h >= 120 && h < 180) {
    r_temp = 0;
    g_temp = c;
    b_temp = x;
  } else if (h >= 180 && h < 240) {
    r_temp = 0;
    g_temp = x;
    b_temp = c;
  } else if (h >= 240 && h < 300) {
    r_temp = x;
    g_temp = 0;
    b_temp = c;
  } else {
    r_temp = c;
    g_temp = 0;
    b_temp = x;
  }
  
  // Scaling the values to the range [0, 255]
  unsigned char r = round((r_temp + m) * 255);
  unsigned char g = round((g_temp + m) * 255);
  unsigned char b = round((b_temp + m) * 255);
  // Serial.println("Color:");
  // Serial.println(r);
  // Serial.println(g);
  // Serial.println(b);

  setRGB(r,g,b);
}

