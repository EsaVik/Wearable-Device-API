#include <Wire.h>

// Board type ('t' for thermistor)
char boardType = 't';

// I2C address for the board (1-127)
byte slaveAddress = 2;

// Input pins for thermistors
byte t1Pin = 2; //thermistor 1 (S1)
byte t2Pin = 3; //thermistor 2 (S2)
byte t3Pin = 8; //thermistor 3 (S3)

// Variables for reading control messages
byte command; // 0-3

// For storing temperature values
// [ thermistor1, thermistor2, thermistor3 ]
int sensorReadings[3];
byte temperatures[3];

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
  pinMode(t1Pin, INPUT);
  pinMode(t2Pin, INPUT);
  pinMode(t3Pin, INPUT);

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
}

// Handles incoming commands
void readEvent(int count) {
  // All control messages start with command type
  command = Wire.read();
  
  // Commands:
  // 0 - Get Type
  // 1 - Read Sensors
  if (command == 0) {
    // No handling needed
  } else if (command == 1) {
    // No handling needed
  }
}

// Handles incoming requests
// 0 - Get Type
// 1 - Read Sensors
void sendEvent() {
  if (command == 0) {
    Wire.write(boardType);
  } else if (command == 1) {
    // Convert temperatures to char array for easy writing
    Wire.write((char*) temperatures, 3);
  }
}

void calculateTemperatures() {
  // Read sensor values
  sensorReadings[0] = analogRead(t1Pin);
  sensorReadings[1] = analogRead(t2Pin);
  sensorReadings[2] = analogRead(t3Pin);
  
  for (int i = 0; i < 3; i++) {
    // Calculate voltage over 10k resistor
    // Supply voltage is 5V and ADC has values between 0-1023
    VR = (VCC / 1023.00) * sensorReadings[i];
    // Calculate voltage over thermistor
    VT = VCC - VR;
    // Calculate resistance caused by temperature
    RT = VT / (VR / R);
    // Calculate temperature in Kelvin based on the measurements
    ln = log(RT / RT0);
    TX = (1 / ((ln / B) + (1 / T0)));
    // Convert to celsius and store
    temperatures[i] = TX - 273.15;
  }
}
