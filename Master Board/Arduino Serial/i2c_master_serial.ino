#include <Wire.h>

// Struct for storing state information, including current command, for peltier slave board
struct peltierBoard {
  byte address;
  long remainingDuration;
  byte temperatureTarget1;
  byte temperatureTarget2;
};

// For keeping track of time since the start of the program
unsigned long elapsedTime = 0;
unsigned long previousTime = 0;

// For storing control messages from serial
String controlMessage;

// For storing information about all peltier slave boards
// TODO: Currently supports only 1
peltierBoard peltier;

// For temporarily storing temperature information retrieved from a slave board
byte temperatures[4];

void setup() {
  // Initialize serial communication for receiving commands
  Serial.begin(9600);
  // Initialize I2C communication with slave boards
  Wire.begin();
  // TODO: Scan for all devices
  peltier = {12, 0, 0, 0};
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
    controlMessage = "";
  }
  
  // TODO: For each peltier board
  // If remainingDuration is 0, the board is either off, or continuing indefinitely
  // If remainingDuration > 0, check whether to switch off the board (duration has run out)
  if (peltier.remainingDuration > 0) {
    peltier.remainingDuration = peltier.remainingDuration - elapsedTime;
    if (peltier.remainingDuration <= 0) {
      peltierSetIntensity(peltier.address, 0, 0, 0, 0);
      peltier.remainingDuration = 0;
      peltier.temperatureTarget1 = 0;
      peltier.temperatureTarget2 = 0;
    }
  }
  // If a temperatureTarget has been set for peltier, adjust pwm accordingly
  // Otherwise, peltier has been set for constant pwm, so let it continue
  if (peltier.temperatureTarget1 > 0) {
    peltierReadSensors(peltier.address);
    // If below set target, heat up, otherwise cool down
    peltierSetIntensity(peltier.address, 255, temperatures[0] < peltier.temperatureTarget1, 255, temperatures[2] < peltier.temperatureTarget2);
  }
  
  // Update timestamps
  elapsedTime = millis() - previousTime;
  previousTime = previousTime + elapsedTime;
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
    // Convert ASCII to 3 digit number
    byte deviceAddress = controlMessage.substring(2,5).toInt();
    if (controlMessage[1] == '0') {
      byte intensity1 = controlMessage.substring(5,8).toInt();
      byte direction1 = controlMessage[8] - '0';
      byte intensity2 = controlMessage.substring(9,12).toInt();
      byte direction2 = controlMessage[13] - '0';
      long duration = controlMessage.substring(14).toInt();
      peltierSetIntensity(deviceAddress, intensity1, direction1, intensity2, direction2);
      Serial.println("Code: 0");
    } else if (controlMessage[1] == '1') {
      // Read sensors
      peltierReadSensors(deviceAddress);
      // Write output to Serial - ts: timestamp, values: [value]
      Serial.print("ts:");
      Serial.print(millis());
      Serial.print("values:[");
      for (int i = 0; i < 3; i++) {
        Serial.print(temperatures[i]);
        Serial.print(",");
      }
      Serial.print(temperatures[3]);
      Serial.println("]");
    }
  }
}

// General API functions

// TODO: Retrieve list of connected I2C devices, along with type
void getDevices() {
  
}

// TODO: Loop over all connected I2C actuators, sending a shutdown signal
void softShutdown() {
  
}

// TODO: Cut power to I2C device power source
void hardShutdown() {
  
}

// TODO: Multiply all temperature intensity values with this multiplier
void setTemperatureMultiplier(float multiplier) {
  
}

// TODO: Multiply all vibration intensity values with this multiplier
void setVibrationMultiplier(float multiplier) {
  
}

// I2C Library for Peltier Slave Board API

// Set Intensity | command intensity1 direction1 intensity2 direction2
void peltierSetIntensity(byte address, byte intensity1, byte direction1, byte intensity2, byte direction2) {
  Wire.beginTransmission(address);
  Wire.write(1);
  Wire.write(intensity1);
  Wire.write(direction1);
  Wire.write(intensity2);
  Wire.write(direction2);
  Wire.endTransmission();
}

// Read Sensors | command
void peltierReadSensors(byte address) {
  Wire.beginTransmission(address);
  Wire.write(2);
  Wire.endTransmission();

  Wire.requestFrom(address, 4);
  for (int i = 0; i < 4; i++) {
    temperatures[i] = Wire.read();
  }
}

// Set Minimum and Maximum temperatures | command minimumTemperatureSide1 maximumTemperatureSide1 minimumTemperatureSide2 maximumTemperatureSide2
void peltierSetIntensity(byte address, byte minimumTemperatureSide1, byte maximumTemperatureSide1, byte minimumTemperatureSide2, byte maximumTemperatureSide2) {
  Wire.beginTransmission(address);
  Wire.write(3);
  Wire.write(minimumTemperatureSide1);
  Wire.write(maximumTemperatureSide1);
  Wire.write(minimumTemperatureSide2);
  Wire.write(maximumTemperatureSide2);
  Wire.endTransmission();
}