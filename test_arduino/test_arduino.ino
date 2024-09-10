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

int fL_trig = 8;
int fL_echo = 9;

int fR_trig = 10;
int fR_echo = 11;

int sL_trig = 18;
int sL_echo = 19;

int sR_trig = 14;
int sR_echo = 15;

long distance_front;
long distance_fL;
long distance_fR;
long distance_side;

long distance_sfront;
long distance_sL;
long distance_sR;
long distance_sside;

int mode = 99; // 모드!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
int middle = 119;

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
  
  pinMode(sL_trig, OUTPUT);
  pinMode(sL_echo, INPUT);
  pinMode(sR_trig, OUTPUT);
  pinMode(sR_echo, INPUT);
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
  else if (message == '1') {
  } else if (message == '2') {
    mode = 0;
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
  distance_sL = ultrasonic_distance(sL_trig, sL_echo);
  distance_sR = ultrasonic_distance(sR_trig, sR_echo);
}

// 메인 루프
void loop() {
  check_mode();
  Serial.print("\n Left: ");
  Serial.print(distance_fL);
  Serial.print(" mm,Right: ");
  Serial.print(distance_fR);
  Serial.print(" mm");
  Serial.print("\n______________________________");
  Serial.print("\n side Left: ");
  Serial.print(distance_sL);
  Serial.print(" mm, Right: ");
  Serial.print(distance_sR);
  Serial.print(" mm\n\n");
  delay(700);



//  //조향 정렬
//    motor_forward(motorH1, motorH2, 135); //왼쪽 조향
//    delay(500);
//    motor_hold(motorH1, motorH2);
//    delay(10000);
   
 }
