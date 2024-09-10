#include <Car_Library.h>

int analogPin = A15; // 가변저항 output pin 

int motorH1 = 7;
int motorH2 = 6;
int motorL1 = 2;
int motorL2 = 3;
int motorR1 = 4;
int motorR2 = 5;

char message = '0';

int direc = 0; // 받아온 방향 
int direction_state = 0;
int reg = 0;
int count = 0;
int flag = 0;
int wheel = 255;
int middle = 105;

void setup() {
  Serial.begin(9600);

  pinMode(motorH1, OUTPUT);
  pinMode(motorH2, OUTPUT);
  pinMode(motorL1, OUTPUT);
  pinMode(motorL2, OUTPUT);
  pinMode(motorR1, OUTPUT);
  pinMode(motorR2, OUTPUT);
}

int potentiometer_Read(int pin) {
  return analogRead(pin);
}

void motor_forward(int motorPin1, int motorPin2, int speed) {
  analogWrite(motorPin1, speed);
  analogWrite(motorPin2, 0);
}

void motor_backward(int motorPin1, int motorPin2, int speed) {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, speed);
}

void motor_hold(int motorPin1, int motorPin2) {
  analogWrite(motorPin1, 0);
  analogWrite(motorPin2, 0);
}

void mescheck(char message) {
  if (message == 'a') direc = 1;
  else if (message == 'b') direc = 3;
  else if (message == 'c') direc = 5;
  else if (message == 'd') direc = 7;
  else if (message == 'e') direc = 9;
  else if (message == 'f') direc = 11; // middle
  else if (message == 'g') direc = 13;
  else if (message == 'h') direc = 15;
  else if (message == 'i') direc = 17;
  else if (message == 'j') direc = 19;
  else if (message == 'k') direc = 21;
  else direc = 0;
}

void check_direc(int a) {
  if (a > middle + 16) direction_state = 1; // L5
  else if (a > middle + 15 && a <= middle + 16) direction_state = 2;
  else if (a > middle + 13 && a <= middle + 15) direction_state = 3; // L4
  else if (a > middle + 11 && a <= middle + 13) direction_state = 4;
  else if (a > middle + 9 && a <= middle + 11) direction_state = 5; // L3
  else if (a > middle + 8 && a <= middle + 9) direction_state = 6;
  else if (a > middle + 6 && a <= middle + 8) direction_state = 7; // L2
  else if (a > middle + 5 && a <= middle + 6) direction_state = 8;
  else if (a > middle + 3 && a <= middle + 5) direction_state = 9; // L1
  else if (a > middle + 2 && a <= middle + 3) direction_state = 10;
  else if (a > middle && a <= middle + 2) direction_state = 11; // middle
  else if (a > middle - 1 && a <= middle) direction_state = 12;
  else if (a > middle - 4 && a <= middle - 2) direction_state = 13; // R1
  else if (a > middle - 5 && a <= middle - 4) direction_state = 14;
  else if (a > middle - 7 && a <= middle - 5) direction_state = 15; // R2
  else if (a > middle - 9 && a <= middle - 8) direction_state = 16;
  else if (a > middle - 12 && a <= middle - 10) direction_state = 17; // R3
  else if (a > middle - 14 && a <= middle - 12) direction_state = 18;
  else if (a > middle - 16 && a <= middle - 14) direction_state = 19; // R4
  else if (a > middle - 17 && a <= middle - 16) direction_state = 20;
  else if (a <= middle - 17) direction_state = 21; // R5
}

void loop() {
  if (Serial.available()) {
    message = Serial.read();
    mescheck(message);
    reg = potentiometer_Read(analogPin);
    check_direc(reg);
    if (message == '2') {
      count = 1;
    } else if (message == '1') {
      count = 2;
    }
  }
  
  if (count == 1) {
    motor_forward(motorL1, motorL2, wheel);
    motor_forward(motorR1, motorR2, wheel);
      
    if (message == '0') {
      count = 0;
      motor_hold(motorL1, motorL2);
      motor_hold(motorR1, motorR2);
    }
  } else if (count == 2) {
    if (message == '0') {
      count = 0;
      motor_hold(motorL1, motorL2);
      motor_hold(motorR1, motorR2);
    }
  }
  
  if (direc > 0) {
    if (direc != direction_state) { 
      if (direc < direction_state) {
        motor_forward(motorH1, motorH2, 255);
        delay(10);
        motor_hold(motorH1, motorH2);
        reg = potentiometer_Read(analogPin);
        check_direc(reg);
      } else if (direc > direction_state) {
        motor_backward(motorH1, motorH2, 255);
        delay(10);
        motor_hold(motorH1, motorH2);
        reg = potentiometer_Read(analogPin);
        check_direc(reg);
      }
    } else {
      motor_hold(motorH1, motorH2);
      reg = potentiometer_Read(analogPin);
      check_direc(reg);
    }
  }
}
