#include <Wire.h>

// For keeping track of time since the start of the program
unsigned long currentTime = 0;
unsigned long elapsedTime = 0;
unsigned long previousTime = 0;

// For storing control messages from serial
String controlMessage;

// For storing information about all peltier slave boards
// TODO: Currently supports only 1
peltierBoard peltier;

// Struct for storing state information, including current command, for peltier slave board
struct peltierBoard {
  byte address;
  long remainingDuration;
  byte temperatureTarget1;
  byte temperatureTarget2;
}

void setup() {
  // Initialize serial communication for receiving commands
  Serial.begin(9600);
  // Initialize I2C communication with slave boards
  Wire.begin();
  // Scan for all devices
}

void loop() {
  // Check for incoming control messages without interrupting control loop
  if (Serial.available()) {
    // Read entire control message to be handled
    while (Serial.available()) {
      char inByte = Serial.read();
      if (inByte != '\n') {
        controlMessage += inByte;
      }
    }
    // Handle control message
    handleMessage();
    controlMessage = '';
  }
  
  // TODO: For each peltier board
  // If remainingDuration is 0, the board is either off, or continuing indefinitely
  // If remainingDuration > 0, check whether to switch off the board (duration has run out)
  if (peltier.remainingDuration > 0) {
    //peltier.remainingDuration = peltier.remainingDuration + 
  }
  
  // Update timestamps
  //currentTime = ;
  //elapsedTime = ;
  //previousTime = ;
}

// Message format:
// First byte - Device type
// Second byte - Command type
// Third/nth byte - Parameters
void handleMessage() {
  // Peltier Device API
  // Commands:
  // 0 - Set Intensity | address intensity1 direction1 intensity2 direction2 duration
  // 1 - Read Sensors | address
  // 2 - Set Minimum and Maximum temperatures | address minimumTemperatureSide1 maximumTemperatureSide1 minimumTemperatureSide2 maximumTemperatureSide2
  // 3 - Set Target Temperature | address target1 target2 duration
  if (controlMessage[0] == 'p') {
    if (controlMessage[1] == '0') {
      // Convert ASCII to 3 digit number
      byte deviceAddress = controlMessage.substring(2,5).toInt();
      byte intensity1 = controlMessage.substring(5,8).toInt();
      byte direction1 = controlMessage[8] - '0';
      byte intensity2 = controlMessage.substring(9,12).toInt();
      byte direction2 = controlMessage[13] - '0';
      long duration = controlMessage.substring(14).toInt();
    }
  }
}

void peltierSetIntensity(byte address, )