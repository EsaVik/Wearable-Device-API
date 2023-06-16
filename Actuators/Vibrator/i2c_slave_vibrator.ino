#include <Wire.h>

// Board type ('v' for vibrator)
char boardType = 'v';

// I2C address for the board (1-127)
byte slaveAddress = 2;

// Output pins for controlling vibration motors
byte v1Pin = 1; // Motor 1
byte v2Pin = 4; // Motor 2

// Variables for reading control messages
byte command; // 0-1
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
    intensity1 = Wire.read();
    intensity2 = Wire.read();
	  duration = 0;
	  for (int i = 0 ; i < 4 ; i++) {
			duration = (duration << 8) + Wire.read();
		}
  }
}

// Handles incoming requests
// 0 - Get Type
void sendEvent() {
	if (command == 0) {
		Wire.write(boardType);
	}
}