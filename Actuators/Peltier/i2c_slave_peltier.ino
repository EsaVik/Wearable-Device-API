#include <Wire.h>

// Board type ('p' for peltier)
char boardType = 'p';

// I2C address for the board (1-127)
byte slaveAddress = 2;

// Output pins for controlling peltier elements
byte p1Pin1 = 1;
byte p1Pin2 = 2;
byte p2Pin1 = 3;
byte p2Pin2 = 4;

// Input pins for thermistors
byte t1Pin = 5;
byte t2Pin = 6;
byte t3Pin = 7;
byte t4Pin = 8;


// Variables for reading control messages
byte command; // 0-3
byte intensity1; // 0-255
byte intensity2; // 0-255
byte direction1; // 0 (heating) or 1 (cooling)
byte direction2; // 0 (heating) or 1 (cooling)
byte maximumTemperatureSide1 = 45; // 0-100
byte maximumTemperatureSide2 = 45; // 0-100
byte minimumTemperatureSide1 = 45; // 0-100
byte minimumTemperatureSide2 = 45; // 0-100

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
float VR, RT, ln, TX;



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
  Wire.onReceive(readEvent);
  
}

void loop() {
  // Update temperature values
  calculateTemperatures();
  
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
}

// Handles incoming commands
void readEvent(int count) {
  // All control messages start with command type
  command = Wire.read();
  
  // Commands:
  // 0 - Get Type
  // 1 - Set Intensity | intensity1 direction1 intensity2 direction2
  // 2 - Read Sensors
  // 3 - Set Minimum and Maximum temperatures | minimumTemperatureSide1 maximumTemperatureSide1 minimumTemperatureSide2 maximumTemperatureSide2
  if (command == 0) {
    Wire.write(boardType);
  } else if (command == 1) {
    intensity1 = Wire.read();
    direction1 = Wire.read();
    intensity2 = Wire.read();
    direction2 = Wire.read();
  } else if (command == 2) {
    intensity1 = Wire.read();
    direction1 = Wire.read();
    intensity2 = Wire.read();
    direction2 = Wire.read();
  } else if (command == 3) {
    minimumTemperatureSide1 = Wire.read();
    maximumTemperatureSide1 = Wire.read();
    minimumTemperatureSide2 = Wire.read();
    maximumTemperatureSide2 = Wire.read();
  }
}

void calculateTemperatures() {
  // Read sensor values
  sensorReadings[0] = analogRead(t1Pin);
  sensorReadings[1] = analogRead(t2Pin);
  sensorReadings[2] = analogRead(t3Pin);
  sensorReadings[3] = analogRead(t4Pin);
  
  for (int i = 0; i < 4; i++) {
    // Convert to voltage
    // Supply voltage is 5V and ADC has values between 0-1023
    sensorReadings[i] = (VCC / 1023.00) * sensorReadings[i];
    VR = VCC - sensorReadings[i];
    // Calculate resistance of RT+
    RT = sensorReadings[i] / (VR / R);
  
    ln = log(RT / RT0);
    TX = (1 / ((ln / B) + (1 / T0)));
  
    // Convert to celsius and store
    temperatures[i] = TX - 273.15;
  }
}