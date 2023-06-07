#include <Wire.h>

byte out1_pin = 1;//output pin
byte out2_pin = 4;//output pin

byte slave_add = 2;//the i2c address of this slave(1-127)

byte pwm_val;//0-255
char dir;//'0' or '1'


void setup() {

  pinMode(out1_pin, OUTPUT);
  pinMode(out2_pin, OUTPUT);

  Wire.begin(slave_add);
  Wire.onReceive(readEvent);

}

void loop() {
  
  //the byte from Wire.read() is char and show in ASCII code (ATtiny412) 
  if (dir == 49) { // dir = 1
    analogWrite(out1_pin, pwm_val);
    analogWrite(out2_pin, 0);
  }
  else if(dir == 48){ //dir = 0
    analogWrite(out1_pin, 0);
    analogWrite(out2_pin, pwm_val);
  }

}

void readEvent(int c) {
  dir = Wire.read(); //first received command is direction
  pwm_val = Wire.read(); //second received command is PWM value
}
