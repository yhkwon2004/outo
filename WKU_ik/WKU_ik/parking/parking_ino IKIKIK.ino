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
int middle = 120; // 가변저항 중앙값
int handle_range = 20; // 핸들 회전 범위

void setup() {
  Serial.begin(9600);
  pinMode(motorH1, OUTPUT);
  pinMode(motorH2, OUTPUT);
  pinMode(motorL1, OUTPUT);
  pinMode(motorL2, OUTPUT);
  pinMode(motorR1, OUTPUT);
  pinMode(motorR2, OUTPUT);
}

void loop() {
  if (Serial.available()) {
    message = Serial.read();
    executeCommand(message);
  }
}

void executeCommand(char cmd) {
  switch (cmd) {
    case 'M':  // 전진
      motor_forward(motorL1, motorL2, wheel);
      motor_forward(motorR1, motorR2, wheel);
      break;
    case 'H':  // 정지
      motor_hold(motorL1, motorL2);
      motor_hold(motorR1, motorR2);
      motor_hold(motorH1, motorH2);
      break;
    case '1':  // 후진
      motor_backward(motorL1, motorL2, wheel/2);
      motor_backward(motorR1, motorR2, wheel/2);
      break;
    case '2':  // 오른쪽으로 돌리며 후진
      handle2(middle - handle_range); // 오른쪽으로 최대한 돌림
      motor_backward(motorL1, motorL2, wheel/2);
      motor_backward(motorR1, motorR2, wheel/3);
      break;
    case '3':  // 왼쪽으로 돌리며 전진
      handle2(middle + handle_range); // 왼쪽으로 최대한 돌림
      motor_forward(motorL1, motorL2, wheel/3);
      motor_forward(motorR1, motorR2, wheel/2);
      break;
    case '4':  // 오른쪽으로 미세 조정
      handle2(middle - handle_range/2); // 오른쪽으로 절반만큼 돌림
      motor_forward(motorL1, motorL2, wheel/3);
      motor_backward(motorR1, motorR2, wheel/3);
      break;
    case '5':  // 왼쪽으로 미세 조정
      handle2(middle + handle_range/2); // 왼쪽으로 절반만큼 돌림
      motor_backward(motorL1, motorL2, wheel/3);
      motor_forward(motorR1, motorR2, wheel/3);
      break;
    case '6':  // 핸들 중앙 정렬
      handle2(middle);
      break;
  }
}

void handle2(int target) {
  reg = potentiometer_Read(analogPin);
  while (abs(reg - target) > 2) {  // 2의 오차 허용
    if (reg > target) {
      motor_backward(motorH1, motorH2, wheel/2);
    } else {
      motor_forward(motorH1, motorH2, wheel/2);
    }
    delay(30);
    reg = potentiometer_Read(analogPin);
  }
  motor_hold(motorH1, motorH2);
}