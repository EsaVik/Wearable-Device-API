#include <Wire.h>

// Board type ('v' for vibrator)
char boardType = 'v';

// I2C address for the board (1-127)
byte slaveAddress = 2;

// Output pins for controlling vibration motors
byte v1Pin = 1; // Motor 1
byte v2Pin = 4; // Motor 2

// Variables for reading control messages
// For concurrency reasons, some are duplicates of program state
byte command; // 0-1
byte newIntensity1 = 0; // 0-255
byte newIntensity2 = 0; // 0-255
long newDuration = 0;
bool updateState = false;

// Variables for program state
byte intensity1 = 0; // 0-255
byte intensity2 = 0; // 0-255
long duration = 0;

// For keeping track of time since the start of the program
unsigned long elapsedTime = 0;
unsigned long previousTime = 0;

void setup() {
  
  // Initialize output pins
  pinMode(v1Pin, OUTPUT);
  pinMode(v2Pin, OUTPUT);

  // Initialize I2C communication
  Wire.begin(slaveAddress);
  // Function for handling incoming commands
  Wire.onReceive(readEvent);
  // Function for handling requests
  Wire.onRequest(sendEvent);
}

void loop() {
  // If a new command has been received, update state
  if (updateState) {
    duration = newDuration;
    intensity1 = newIntensity1;
    intensity2 = newIntensity2;
    updateState = false;
  }
  
  // If duration is 0, the board is either off, or continuing indefinitely
  // If duration > 0, check whether to switch off the board (duration has run out)
  if (duration > 0) {
    duration = duration - elapsedTime;
    if (duration <= 0) {
      intensity1 = 0;
      intensity2 = 0;
    }
  }
  
  analogWrite(v1Pin, intensity1);
  analogWrite(v2Pin, intensity2);
  
  // Update timestamps
  elapsedTime = millis() - previousTime;
  previousTime = previousTime + elapsedTime;
}

// Handles incoming commands
void readEvent(int count) {
  // All control messages start with command type
  command = Wire.read();
  
  // Commands:
  // 0 - Get Type
  // 1 - Set Intensity | intensity1 intensity2 duration
  if (command == 0) {
    // No handling needed
  } else if (command == 1) {
    newIntensity1 = Wire.read();
    newIntensity2 = Wire.read();
    newDuration = 0;
    // Store all 4 bytes of duration in a long int
    for (int i = 0 ; i < 4 ; i++) {
      long temporaryVariable = 0;
      // Read byte and move it to correct position
      temporaryVariable = (temporaryVariable | Wire.read()) << (8 * i);
      // Store byte in correct position in newDuration
      newDuration = newDuration | temporaryVariable;
    }
    updateState = true;
  }
}

// Handles incoming requests
// 0 - Get Type
void sendEvent() {
  if (command == 0) {
    Wire.write(boardType);
  }
}
