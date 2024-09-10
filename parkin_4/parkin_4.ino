#include <Car_Library.h>

int analogPin = A15; // 가변저항 output pin

int motorH1 = 7;
int motorH2 = 6;
int motorL1 = 3;
int motorL2 = 2;
int motorR1 = 4;
int motorR2 = 5;

char message = '0';
int direc = 11; // 받아온 방향
int direction_state;
int reg = 0;

int fL_trig = 14;
int fL_echo = 15;

int fR_trig = 18;
int fR_echo = 19;

long distance_front;
long distance_fL;
long distance_fR;
long distance_side;

int mode = 99; // 모드!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
int middle = 119;
int time1 = 0;



void setup() {
  Serial.begin(9600);
  pinMode(motorH1, OUTPUT);
  pinMode(motorH2, OUTPUT);
  pinMode(motorL1, OUTPUT);
  pinMode(motorL2, OUTPUT);
  pinMode(motorR1, OUTPUT);
  pinMode(motorR2, OUTPUT);

  pinMode(fL_trig, OUTPUT);
  pinMode(fL_echo, INPUT);
  pinMode(fR_trig, OUTPUT);
  pinMode(fR_echo, INPUT);
}


void setDirecToMiddle() {
  direc = 11;
}



void mescheck(char message) {
  if (message == 'a') direc = 1;
  else if (message == 'b') direc = 3;
  else if (message == 'c') direc = 5;
  else if (message == 'd') direc = 7;
  else if (message == 'e') direc = 9;
  else if (message == 'f') direc = 11;
  else if (message == 'g') direc = 13;
  else if (message == 'h') direc = 15;
  else if (message == 'i') direc = 17;
  else if (message == '1') { // 가까운점 1번 시작
    mode = 0;
    time1 = 9000;
  } else if (message == '2') { // 멀리있는점 2번시작
    mode = 0;
    time1 = 11500;
  }
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
  else if (a > middle - 12 && a <= middle - 10) direction_state = 19; // R4
  else if (a > middle - 13 && a <= middle - 12) direction_state = 20;
  else if (a <= middle - 13) direction_state = 21; // R5
}

void check_mode() { // 초음파 로직
  distance_fL = ultrasonic_distance(fL_trig, fL_echo);
  distance_fR = ultrasonic_distance(fR_trig, fR_echo);

  if (distance_fL != 0 && distance_fR != 0) {
    if ((distance_fL - 70 <= distance_fR) && (distance_fL + 70) >= distance_fR) {
      distance_front = (distance_fL + distance_fR) / 2;
    } else if ((distance_fR - 70 <= distance_fL) && (distance_fR + 70) >= distance_fL) {
      distance_front = (distance_fL + distance_fR) / 2;
    } else distance_front = 2000;
  } else distance_front = 2000;
}

// 메인 루프
void loop() {
  check_mode();
  Serial.print("Left Distance: ");
  Serial.print(distance_fL);
  Serial.print(" cm, Right Distance: ");
  Serial.print(distance_fR);
  Serial.print(" cm, Front Distance: ");
  Serial.print(distance_front);
  Serial.println(" cm");

  // 신호 대기용도
  if (Serial.available() > 0) {
    message = Serial.read();
    mescheck(message);
  }

  if (direc > 0) {
    while (direc != direction_state) {
      if (direc < direction_state) {
        motor_forward(motorH1, motorH2, 255);
        delay(40);
      } else if (direc > direction_state) {
        motor_backward(motorH1, motorH2, 255);
        delay(40);
      }
      motor_hold(motorH1, motorH2);
      reg = potentiometer_Read(analogPin);
      check_direc(reg);
    }
    motor_hold(motorH1, motorH2);
  }

  if (mode == 0) { // 모드 0: 직진 및 전진 후 모드 2로 전환
    direc = 11;
    delay(500);

    motor_forward(motorL1, motorL2, 150);
    motor_forward(motorR1, motorR2, 150);

    check_mode();
    if (distance_front <= 1100) {
      motor_hold(motorL1, motorL2);
      motor_hold(motorR1, motorR2);
      delay(1000);
      mode = 2;
    }
  }

  if (mode == 2) { // 모드 2: 복합 동작 후 모드 3으로 전환  직진
    motor_forward(motorL1, motorL2,150);
    motor_forward(motorR1, motorR2,150); 
    delay(1000);                      ////////////    감지된후 얼마나 더 앞으로 갈지.!!!!!!!!!!!!!!!!!!!!!!!!!!!!1
    motor_hold(motorL1, motorL2);
    motor_hold(motorR1, motorR2);
    delay(2000);
    motor_forward(motorH1, motorH2, 170); //왼쪽 조향
    delay(1000);
    motor_hold(motorH1, motorH2);
    motor_hold(motorL1, motorL2);
    motor_hold(motorR1, motorR2);
    delay(2000);
    motor_forward(motorL1, motorL2, 70); //왼쪽 상태에서 직진 8초
    motor_forward(motorR1, motorR2, 40);
    delay(8000);
    motor_hold(motorL1, motorL2);
    motor_hold(motorR1, motorR2);
    motor_backward(motorH1, motorH2, 170); //오른쪽 조향
    delay(1000);
    motor_hold(motorH1, motorH2);
    motor_backward(motorL1, motorL2, 70);
    motor_backward(motorR1, motorR2, 100);  //후진
    delay(4100);
    motor_hold(motorL1, motorL2);
    motor_hold(motorR1, motorR2);
    delay(1000);
    mode = 3;
    direc = 11;
  }

  if (mode == 3) {
    motor_forward(motorH1, motorH2, 180); //왼쪽 조향
    delay(525);
    motor_hold(motorH1, motorH2);
    mode = 4;
    direc = 11;
 }

    if (mode == 4) { // 모드 4: 후진 및 전진 후 모드 5로 전환   // 정렬된 차량을 뒤로 후진
      delay(2000);
      motor_backward(motorL1, motorL2, 50);           // 정렬후 후진!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      motor_backward(motorR1, motorR2, 50);
      delay(time1);
      motor_hold(motorL1, motorL2);
      motor_hold(motorR1, motorR2);
      delay(4000);                             //주차후 정지 

      
     
      motor_forward(motorL1, motorL2, 70);    // 탈출
      motor_forward(motorR1, motorR2, 70);
      delay(2000);
      motor_hold(motorL1, motorL2);
      motor_hold(motorR1, motorR2);
      motor_backward(motorH1, motorH2, 170);  //오른쪽
      delay(3000);
      motor_hold(motorH1, motorH2);
      delay(1000);
      motor_forward(motorL1, motorL2, 70); //오른쪽 상태에서 직진
      motor_forward(motorR1, motorR2, 70);
      delay(9650); //  오른쪽으로 얼마나 갈지  각도 수정!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      motor_hold(motorL1, motorL2);
      motor_hold(motorR1, motorR2);
      delay(1000);
      mode = 5;
     }

  if (mode == 5) {  //조향 정렬
    motor_forward(motorH1, motorH2, 180); //왼쪽 조향
    delay(525);
    motor_hold(motorH1, motorH2);
    mode = 6;
    direc = 11;
   
 }
 
if(mode==6){    //직진으로 탈출 끝
  motor_forward(motorL1, motorL2, 70);
  motor_forward(motorR1, motorR2, 70);
   delay(10000);
}
} 
