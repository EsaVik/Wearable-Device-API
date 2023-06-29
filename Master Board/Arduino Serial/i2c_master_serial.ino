#include <Wire.h>

// For storing control messages from serial
String controlMessage;

// For temporarily storing temperature information retrieved from a slave board
byte temperatures[4];

// For storing all connected boards
char boards[128] = {0};

// For storing intensityMultipliers
float temperatureMultiplier = 1;
float vibrationMultiplier = 1;

void setup() {
  // Initialize serial communication for receiving commands
  Serial.begin(9600);
  // Initialize I2C communication with slave boards
  Wire.begin();
  // Scan for all devices
  getDevices();
}

void loop() {
  // Check for incoming control messages without interrupting control loop
  if (Serial.available()) {
    char inByte = Serial.read();
    // Read entire control message to be handled
    while (inByte != '\n') {
      if (Serial.available()) {
        controlMessage += inByte;
        inByte = Serial.read();
      }
    }

    // Handle control message
    handleMessage();
    controlMessage = "";
  }
  
  // Any required control functionality
  
}

// ############
// #Serial API#
// ############

// Message format:
// First byte - Device type
// Second byte - Command type
// Third/nth byte - Parameters
void handleMessage() {
  // General API
  // Commands:
  // 0 - Get Devices
  // 1 - Soft Shutdown
  // 2 - Hard Shutdown
  // 3 - Set Temperature Multiplier | multiplier
  // 4 - Set Vibration Multiplier | multiplier
  if (controlMessage[0] == 'g') {
    if (controlMessage[1] == '0') {
      bool first = true;
      Serial.print("devices:[");
      for (int i = 1; i < 128; i++) {
        if (boards[i]) {
          if (!first) {
            Serial.print(',');
          }
          Serial.print('(');
          Serial.print(i);
          Serial.print(',');
          Serial.print(boards[i]);
          Serial.print(')');
          first = false;
        }
      }
      Serial.println(']');
    } else if (controlMessage[1] == '1') {
      softShutdown();
      Serial.println("code: 0");
    } else if (controlMessage[1] == '2') {
      hardShutdown();
      Serial.println("code: 0");
    } else if (controlMessage[1] == '3') {
      float multiplier = controlMessage.substring(4).toFloat();
      setTemperatureMultiplier(multiplier);
      Serial.println("code: 0");
    }  else if (controlMessage[1] == '4') {
      float multiplier = controlMessage.substring(4).toFloat();
      setVibrationMultiplier(multiplier);
      Serial.println("code: 0");
    }
  // Peltier Device API
  // Commands:
  // 0 - Set Intensity | address intensity1 direction1 intensity2 direction2 duration
  // 1 - Read Sensors | address
  // 2 - Set Minimum and Maximum temperatures | address minimumTemperatureSide1 maximumTemperatureSide1 minimumTemperatureSide2 maximumTemperatureSide2
  // 3 - Set Target Temperature | address target1 target2 duration
  } else if (controlMessage[0] == 'p') {
    // Convert ASCII to 3 digit number
    byte deviceAddress = controlMessage.substring(2,5).toInt();
    if (controlMessage[1] == '0') {
      byte intensity1 = controlMessage.substring(5,8).toInt();
      byte direction1 = controlMessage[8] - '0';
      byte intensity2 = controlMessage.substring(9,12).toInt();
      byte direction2 = controlMessage[12] - '0';
      long duration = controlMessage.substring(13).toInt();
      peltierSetIntensity(deviceAddress, intensity1, direction1, intensity2, direction2, duration);
      Serial.println("code: 0");
    } else if (controlMessage[1] == '1') {
      // Read sensors
      peltierReadSensors(deviceAddress);
      // Write output to Serial - ts: timestamp, values: [value]
      Serial.print("ts:");
      Serial.print(millis());
      Serial.print(",values:[");
      for (int i = 0; i < 3; i++) {
        Serial.print(temperatures[i]);
        Serial.print(",");
      }
      Serial.print(temperatures[3]);
      Serial.println("]");
    } else if (controlMessage[1] == '2') {
      byte minimumTemperatureSide1 = controlMessage.substring(5,8).toInt();
      byte maximumTemperatureSide1 = controlMessage.substring(8,11).toInt();
      byte minimumTemperatureSide2 = controlMessage.substring(11,14).toInt();
      byte maximumTemperatureSide2 = controlMessage.substring(14,17).toInt();
      peltierSetMinimumMaximum(deviceAddress, minimumTemperatureSide1, maximumTemperatureSide1, minimumTemperatureSide2, maximumTemperatureSide2);
      Serial.println("code: 0");
    }  else if (controlMessage[1] == '3') {
      byte temperatureTarget1 = controlMessage.substring(5,8).toInt();
      byte temperatureTarget2 = controlMessage.substring(8,11).toInt();
      long duration = controlMessage.substring(11).toInt();
      peltierSetTarget(deviceAddress, temperatureTarget1, temperatureTarget2, duration);
      Serial.println("code: 0");
    }
  // Vibrator Device API
  // Commands:
  // 0 - Set Intensity | address intensity1 intensity2 duration
  } else if (controlMessage[0] == 'v') {
    // Convert ASCII to 3 digit number
    byte deviceAddress = controlMessage.substring(2,5).toInt();
    if (controlMessage[1] == '0') {
      byte intensity1 = controlMessage.substring(5,8).toInt();
      byte intensity2 = controlMessage.substring(8,11).toInt();
      long duration = controlMessage.substring(11).toInt();
      vibratorSetIntensity(deviceAddress, intensity1, intensity2, duration);
      Serial.println("code: 0");
    }
  }
}

// General API functions

// Retrieve list of connected I2C devices, along with type
void getDevices() {
  for (int i = 0; i < 128; i++) {
    Wire.beginTransmission(i);
    byte error = Wire.endTransmission();
 
    if (error == 0) {
      // Store device address and type
      boards[i] = i2cGetType(i);
    } else {
      boards[i] = 0;
    }
  }
}

// Loop over all connected I2C actuators, sending a shutdown signal
void softShutdown() {
  for (int i = 0; i < 128; i++) {
    if (boards[i] == 'p') {
      peltierSetIntensity(i, 0, 0, 0, 0, 0);
    } else if (boards[i] == 'v') {
      vibratorSetIntensity(i, 0, 0, 0);
    }
  }
}

// TODO: Cut power to I2C device power source
void hardShutdown() {
  
}

// TODO: Multiply all temperature intensity values with this multiplier
void setTemperatureMultiplier(float multiplier) {
  temperatureMultiplier = multiplier;
}

// TODO: Multiply all vibration intensity values with this multiplier
void setVibrationMultiplier(float multiplier) {
  vibrationMultiplier = multiplier;
}

// #######################
// #I2C Library Functions#
// #######################

// General I2C Library

// Get Type | command
char i2cGetType(byte address) {
  Wire.beginTransmission(address);
  Wire.write(0);
  Wire.endTransmission();
  
  Wire.requestFrom((int) address, 1);
  while (true) {
    if (Wire.available()) {
      return Wire.read();
    }
  }
}

// I2C Library for Peltier Slave Board API

// Set Intensity | command intensity1 direction1 intensity2 direction2 duration
void peltierSetIntensity(byte address, byte intensity1, byte direction1, byte intensity2, byte direction2, long duration) {
  Wire.beginTransmission(address);
  Wire.write(1);
  Wire.write(intensity1);
  Wire.write(direction1);
  Wire.write(intensity2);
  Wire.write(direction2);
  Wire.write((char*) &duration, 4);
  Wire.endTransmission();
}

// Read Sensors | command
void peltierReadSensors(byte address) {
  Wire.beginTransmission(address);
  Wire.write(2);
  Wire.endTransmission();
  
  Wire.requestFrom((int) address, 4);
  int i = 0;
  while (i < 4) {
    if (Wire.available()) {
      temperatures[i] = Wire.read();
      i++;
    }
  }
}

// Set Minimum and Maximum temperatures | command minimumTemperatureSide1 maximumTemperatureSide1 minimumTemperatureSide2 maximumTemperatureSide2
void peltierSetMinimumMaximum(byte address, byte minimumTemperatureSide1, byte maximumTemperatureSide1, byte minimumTemperatureSide2, byte maximumTemperatureSide2) {
  Wire.beginTransmission(address);
  Wire.write(3);
  Wire.write(minimumTemperatureSide1);
  Wire.write(maximumTemperatureSide1);
  Wire.write(minimumTemperatureSide2);
  Wire.write(maximumTemperatureSide2);
  Wire.endTransmission();
}

// Set Minimum and Maximum temperatures | command target1 target2 duration
void peltierSetTarget(byte address, byte temperatureTarget1, byte temperatureTarget2, long duration) {
  Wire.beginTransmission(address);
  Wire.write(4);
  Wire.write(temperatureTarget1);
  Wire.write(temperatureTarget2);
  Wire.write((char*) &duration, 4);
  Wire.endTransmission();
}

// I2C Library for Vibrator Slave Board API

// Set Intensity | command intensity1 intensity2 duration
void vibratorSetIntensity(byte address, byte intensity1, byte intensity2, long duration) {
  Wire.beginTransmission(address);
  Wire.write(1);
  Wire.write(intensity1);
  Wire.write(intensity2);
  Wire.write((char*) &duration, 4);
  Wire.endTransmission();
}
