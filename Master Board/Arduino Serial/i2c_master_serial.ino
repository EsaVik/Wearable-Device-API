#include <Wire.h>

// For storing control messages from serial
String controlMessage;

// For temporarily storing temperature information retrieved from a slave board
byte temperatures[4];

void setup() {
  // Initialize serial communication for receiving commands
  Serial.begin(9600);
  // Initialize I2C communication with slave boards
  Wire.begin();
  // TODO: Scan for all devices
  
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
      peltierSetIntensity(deviceAddress, intensity1, direction1, intensity2, direction2, duration);
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
      Serial.println("Code: 0");
    }
  }
}

// General API functions

// TODO: Retrieve list of connected I2C devices, along with type
void getDevices() {
  for (int i = 1 ; i < 128; i++) {
		Wire.beginTransmission(i);
		byte error = Wire.endTransmission();
 
    if (error == 0) {
			// TODO: Store device address and type
		}
	}
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