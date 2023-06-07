#include <Wire.h>

// I2C address for the board (1-127)
byte slave_add = 2;

// Output pins for controlling peltier elements
byte p1_pin1 = 1;
byte p1_pin2 = 2;
byte p2_pin1 = 3;
byte p2_pin2 = 4;

// Input pins for thermistors
byte t1_pin = 5;
byte t2_pin = 6;
byte t3_pin = 7;
byte t4_pin = 8;


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
float sensorReadings[4];
float temperatures[4];

// For calculating temperature
int VCC = 5; // Supply voltage
int R = 10000; // R = 10KΩ
float T0 = 298.15; // T0 in Kelvin
float VR, RT, ln, TX;



void setup() {
	
  // Initialize input and output pins
  pinMode(p1_pin1, OUTPUT);
  pinMode(p1_pin2, OUTPUT);
  pinMode(p2_pin1, OUTPUT);
  pinMode(p2_pin2, OUTPUT);
  
  pinMode(t1_pin, INPUT);
  pinMode(t2_pin, INPUT);
  pinMode(t3_pin, INPUT);
  pinMode(t4_pin, INPUT);

  // Initialize I2C communication
  Wire.begin(slave_add);
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
		analogWrite(p1_pin1, 0);
		analogWrite(p1_pin2, 0);
	  } else {
		analogWrite(p1_pin1, intensity1);
		analogWrite(p1_pin2, 0);
	  }
  // Cooling side 1, heating side 2
  } else {
	if (temperatures[0] < minimumTemperatureSide1 || temperatures[1] > maximumTemperatureSide2) {
		analogWrite(p1_pin1, 0);
		analogWrite(p1_pin2, 0);
	  } else {
		analogWrite(p1_pin1, 0);
		analogWrite(p1_pin2, intensity1);
	  }
  }
  
  // Peltier2
  // Heating side 1, cooling side 2
  if (direction2 == 0) {
	  if (temperatures[2] > maximumTemperatureSide1 || temperatures[3] < minimumTemperatureSide2) {
		analogWrite(p2_pin1, 0);
		analogWrite(p2_pin2, 0);
	  } else {
		analogWrite(p2_pin1, intensity1);
		analogWrite(p2_pin2, 0);
	  }
  // Cooling side 1, heating side 2
  } else {
	if (temperatures[2] < minimumTemperatureSide1 || temperatures[3] > maximumTemperatureSide2) {
		analogWrite(p2_pin1, 0);
		analogWrite(p2_pin2, 0);
	  } else {
		analogWrite(p2_pin1, 0);
		analogWrite(p2_pin2, intensity1);
	  }
  }
}

// Handles incoming commands
void readEvent(int count) {
  command = Wire.read(); // All control messages start with command type
  
  // Commands:
  // 0 - Get Type
  // 1 - Set Intensity | intensity1 direction1 intensity2 direction2
  // 2 - Read Sensors
  // 3 - Set Minimum and Maximum temperatures | minimumTemperatureSide1 maximumTemperatureSide1 minimumTemperatureSide2 maximumTemperatureSide2
  if (command == 0) {
    Wire.write('p');
  } else if (command == 1) {
	intensity1 = Wire.read();
	direction1 = Wire.read();
	intensity2 = Wire.read();
	direction2 = Wire.read();
  } else if (command == 2) {
	Wire.write(temperatures);
  } else if (command == 3) {
	minimumTemperatureSide1 = Wire.read();
	maximumTemperatureSide1 = Wire.read();
	minimumTemperatureSide2 = Wire.read();
	maximumTemperatureSide2 = Wire.read();
  }
}

void calculateTemperatures() {
  // Read sensor values
  sensorReadings[0] = analogRead(t1_pin);
  sensorReadings[1] = analogRead(t2_pin);
  sensorReadings[2] = analogRead(t3_pin);
  sensorReadings[3] = analogRead(t4_pin);
  
  // Thermistor properties
  // RT0: 10 000 Ω
  // T0: 25 C
  // B: 3977 K +- 0.75%
  
  for (int i = 0; i < 4; i++) {
	// Convert to voltage
	// Supply voltage is 5V and ADC has values between 0-1023
    sensorReadings[i] = (5.00 / 1023.00) * sensorReadings[i];
	VR = VCC - VRT;
	// Calculate resistance of RT+
	RT = VRT / (VR / R);
	
	ln = log(RT / RT0);
	TX = (1 / ((ln / 3977) + (1 / T0)));
	
	// Convert to celsius and store
	temperatures[i] = TX - 273.15;
  }
}