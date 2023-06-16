#include <Wire.h>

// Board type ('p' for peltier)
char boardType = 'p';

// I2C address for the board (1-127)
byte slaveAddress = 2;

// Output pins for controlling peltier elements
byte p1Pin1 = 0; //peltier 1 (P1)
byte p1Pin2 = 1; //peltier 1 (P1)
byte p2Pin1 = 5; //peltier 2 (P2)
byte p2Pin2 = 10; //peltier 2 (P2)

// Input pins for thermistors
byte t1Pin = 2; //thermistor 1 (S1)
byte t2Pin = 3; //thermistor 2 (S2)
byte t3Pin = 8; //thermistor 3 (S3)
byte t4Pin = 9; //thermistor 3 (S4)

// Variables for reading control messages
byte command; // 0-3
byte intensity1 = 0; // 0-255
byte intensity2 = 0; // 0-255
byte direction1 = 0; // 0 (heating) or 1 (cooling)
byte direction2 = 0; // 0 (heating) or 1 (cooling)
long duration = 0;
byte maximumTemperatureSide1 = 45; // 0-100
byte maximumTemperatureSide2 = 45; // 0-100
byte minimumTemperatureSide1 = 45; // 0-100
byte minimumTemperatureSide2 = 45; // 0-100
byte temperatureTarget1 = 0;
byte temperatureTarget2 = 0;

// For keeping track of time since the start of the program
unsigned long elapsedTime = 0;
unsigned long previousTime = 0;

// For storing temperature values
// [ Peltier1_Side1, Peltier1_Side2, Peltier2_Side1, Peltier2_Side2 ]
int sensorReadings[4];
byte temperatures[4];

// For calculating temperature
// Thermistor properties
// RT0: 10 000 Ω
// T0: 25 C
// B: 3977 K +- 0.75%

float VCC = 5.00; // Supply voltage
int R = 10000; // R = 10KΩ
int RT0 = 10000; // RT0 = 10KΩ
int B = 3977; // B = 3977
float T0 = 298.15; // T0 in Kelvin
float VR, VT, RT, ln, TX;



void setup() {
  
  // Initialize input and output pins
  pinMode(p1Pin1, OUTPUT);
  pinMode(p1Pin2, OUTPUT);
  pinMode(p2Pin1, OUTPUT);
  pinMode(p2Pin2, OUTPUT);
  
  pinMode(t1Pin, INPUT);
  pinMode(t2Pin, INPUT);
  pinMode(t3Pin, INPUT);
  pinMode(t4Pin, INPUT);

  // Initialize I2C communication
  Wire.begin(slaveAddress);
  // Function for handling incoming commands
  Wire.onReceive(readEvent);
  // Function for handling requests
  Wire.onRequest(sendEvent);
}

void loop() {
  // Update temperature values
  calculateTemperatures();

  // If duration is 0, the board is either off, or continuing indefinitely
  // If duration > 0, check whether to switch off the board (duration has run out)
  if (duration > 0) {
    duration = duration - elapsedTime;
    if (duration <= 0) {
      intensity1 = 0;
      intensity2 = 0;
      temperatureTarget1 = 0;
      temperatureTarget2 = 0;
    }
  }

  // If a temperatureTarget has been set for peltier, adjust pwm accordingly
  // Otherwise, peltier has been set for constant pwm, so let it continue
  if (temperatureTarget1 > 0) {
    // If below set target, heat up, otherwise cool down
    intensity1 = 255;
    intensity2 = 255;
    direction1 = temperatures[0] < temperatureTarget1;
    direction2 = temperatures[2] < temperatureTarget2;
  }
  
  // Peltier1
  // Heating side 1, cooling side 2
  if (direction1 == 0) {
    // If overheating, shut down pwm
    if (temperatures[0] > maximumTemperatureSide1 || temperatures[1] < minimumTemperatureSide2) {
      analogWrite(p1Pin1, 0);
      analogWrite(p1Pin2, 0);
    } else {
      analogWrite(p1Pin1, intensity1);
      analogWrite(p1Pin2, 0);
    }
  // Cooling side 1, heating side 2
  } else {
    if (temperatures[0] < minimumTemperatureSide1 || temperatures[1] > maximumTemperatureSide2) {
      analogWrite(p1Pin1, 0);
      analogWrite(p1Pin2, 0);
    } else {
      analogWrite(p1Pin1, 0);
      analogWrite(p1Pin2, intensity1);
    }
  }
  
  // Peltier2
  // Heating side 1, cooling side 2
  if (direction2 == 0) {
    if (temperatures[2] > maximumTemperatureSide1 || temperatures[3] < minimumTemperatureSide2) {
      analogWrite(p2Pin1, 0);
      analogWrite(p2Pin2, 0);
    } else {
      analogWrite(p2Pin1, intensity1);
      analogWrite(p2Pin2, 0);
    }
  // Cooling side 1, heating side 2
  } else {
    if (temperatures[2] < minimumTemperatureSide1 || temperatures[3] > maximumTemperatureSide2) {
      analogWrite(p2Pin1, 0);
      analogWrite(p2Pin2, 0);
    } else {
      analogWrite(p2Pin1, 0);
      analogWrite(p2Pin2, intensity1);
    }
  }
  
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
  // 1 - Set Intensity | intensity1 direction1 intensity2 direction2 duration
  // 2 - Read Sensors
  // 2 - Set Minimum and Maximum temperatures | minimumTemperatureSide1 maximumTemperatureSide1 minimumTemperatureSide2 maximumTemperatureSide2
  // 3 - Set Temperature Targets | temperatureTarget1 temperatureTarget2 duration
  if (command == 0) {
		// No handling needed
  } else if (command == 1) {
    intensity1 = Wire.read();
    direction1 = Wire.read();
    intensity2 = Wire.read();
    direction2 = Wire.read();
	  duration = 0;
	  for (int i = 0 ; i < 4 ; i++) {
			duration = (duration << 8) + Wire.read();
		}
  } else if (command == 2) {
		// No handling needed
  } else if (command == 3) {
    minimumTemperatureSide1 = Wire.read();
    maximumTemperatureSide1 = Wire.read();
    minimumTemperatureSide2 = Wire.read();
    maximumTemperatureSide2 = Wire.read();
  } else if (command == 4) {
    temperatureTarget1 = Wire.read();
    temperatureTarget2 = Wire.read();
    for (int i = 0 ; i < 4 ; i++) {
		  duration = (duration << 8) + Wire.read();
	  }
  }
}

// Handles incoming requests
// 0 - Get Type
// 2 - Read Sensors
void sendEvent() {
	if (command == 0) {
		Wire.write(boardType);
	} else if (command == 2) {
		// Convert temperatures to char array for easy writing
		Wire.write((char*) temperatures, 4);
	}
}

void calculateTemperatures() {
  // Read sensor values
  sensorReadings[0] = analogRead(t1Pin);
  sensorReadings[1] = analogRead(t2Pin);
  sensorReadings[2] = analogRead(t3Pin);
  sensorReadings[3] = analogRead(t4Pin);
  
  for (int i = 0; i < 4; i++) {
    // Calculate voltage over 10k resistor
    // Supply voltage is 5V and ADC has values between 0-1023
    VR = (VCC / 1023.00) * sensorReadings[i];
		// Calculate voltage over thermistor
    VT = VCC - VR;
    // Calculate resistance caused by temperature
    RT = sensorReadings[i] / (VR / R);
    // Calculate temperature in Kelvin based on the measurements
    ln = log(RT / RT0);
    TX = (1 / ((ln / B) + (1 / T0)));
    // Convert to celsius and store
    temperatures[i] = TX - 273.15;
  }
}
