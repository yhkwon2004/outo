#include <Car_Library.h>

int analogPin = A15; //가변저항 output pin 

int motorH1 = 7;
int motorH2 = 6;
int motorL1 = 3;
int motorL2 = 2;
int motorR1 = 4;
int motorR2 = 5;

int red_lock;

char message = '0';

int direc=11; // 받아온 방향 
int direction_state;
int reg=0;

int fL_trig =8;
int fL_echo =9;

int fR_trig =10;
int fR_echo = 11;

long distance_front;
long distance_fL;
long distance_fR;
long distance_side;

int mode = 5;  // 현재 모드!!!!!!!!!!!!!!!!!!!!!!!!!
int cnt = 0;
int start = 0;
int cnting = 0;
unsigned long startTime; // 시작 시간을 저장하는 변수
unsigned long currentTime; // 현재 시간을 저장하는 변수
const unsigned long delayTime = 3000; // 1000 == 1초를 밀리초로 표현

int middle = 120;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  red_lock = 1;
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

void mescheck(char message){
  if (message == 'a') direc = 1; 
  else if (message == 'b') direc = 3;
  else if (message == 'c') direc = 5;
  else if (message == 'd') direc = 7;
  else if (message == 'e') direc = 9;
  else if (message == 'f') direc = 11;
  else if (message == 'g') direc = 13;
  else if (message == 'h') direc = 15;
  else if (message == 'i') direc = 17; 
  else if (message == 'j') red_lock = 0;
  else if (message == 'k') red_lock = 1;
  else if (message == '1')
  {
    start = 0;
    startTime = millis(); 
  }
  else if (message == '2')
  {
    start = 1; 
    startTime = millis();
  }
  else direc = 0;
}


void check_direc(int a){
    if (a > middle+16) direction_state = 1; //L5
    else if (a > middle+15 and a <= middle+16) direction_state = 2;
    else if (a > middle+13 and a <= middle+15) direction_state = 3; //L4
    else if (a > middle+11 and a <= middle+13) direction_state = 4;
    else if (a > middle+9 and a <= middle+11) direction_state = 5; //L3
    else if (a > middle+8 and a <= middle+9) direction_state = 6;
    else if (a > middle+6 and a <= middle+8) direction_state = 7; //L2
    else if (a > middle+5 and a <= middle+6) direction_state = 8;
    else if (a > middle+3 and a <= middle+5) direction_state = 9; //L1
    else if (a > middle+2 and a <= middle+3) direction_state = 10;
    else if (a > middle and a <= middle+2) direction_state = 11; //middle
    else if (a > middle-1 and a <= middle) direction_state = 12;
    else if (a > middle-4 and a <= middle-2) direction_state = 13; //R1
    else if (a > middle-5 and a <= middle-4) direction_state = 14;
    else if (a > middle-7 and a <= middle-5) direction_state = 15; //R2
    else if (a > middle-9 and a <= middle-8) direction_state = 16;
    else if (a > middle-12 and a <= middle-10) direction_state = 17; //R3
    else if (a > middle-14 and a <= middle-12) direction_state = 18;
    else if (a > middle-12 and a <= middle-10) direction_state = 19; //R4
    else if (a > middle-13 and a <= middle-12) direction_state = 20;
    else if (a <= middle-13) direction_state = 21; //R5
}

void check_mode(){  //초음파 로직
  distance_fL = ultrasonic_distance(fL_trig, fL_echo);
  distance_fR = ultrasonic_distance(fR_trig, fR_echo);
  
  // 왼쪽 오른쪽 값 비교해서 오차가 적을 때 distance_front을 유효한 값으로 읽음 
  if (distance_fL != 0 and distance_fR != 0)
  {
    if ((distance_fL - 70 <= distance_fR) and (distance_fL + 70) >= distance_fR) {
      distance_front = (distance_fL + distance_fR)/2;  
    }
    else if ((distance_fR - 70 <= distance_fL) and (distance_fR + 70) >= distance_fL){
      distance_front = (distance_fL + distance_fR)/2;  
    }
    else distance_front = 2000;
  }
  else distance_front = 2000;
}

void serialEvent(){
  while(Serial.available()) {
    if (mode == 0)
    {
      message = Serial.read();
    }
    else if (mode == 2) {
      message = Serial.read();
    }
    else if (mode == 4){
      message = Serial.read();
    }
    else if (mode == 5){
      message = Serial.read();
    }
    else if (mode == 7){
      message = Serial.read();
    }
  }
}

//////////////////////////////////////////////////////
/*                                                  */
/*                    main loop                     */
/*                                                  */
//////////////////////////////////////////////////////

void loop() {
  check_mode();
  Serial.print("Left Distance: ");
  Serial.print(distance_fL);
  Serial.print(" mm, Right Distance: ");
  Serial.print(distance_fR);
  Serial.print(" mm, Front Distance: ");
  Serial.print(distance_front);
  Serial.println(" mm");
  if (direc > 0)
    {
      if (direc != direction_state) 
      { 
        if (direc < direction_state){
            motor_forward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
          }
        else if (direc > direction_state){
            motor_backward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
        }
      }
      else
      {
        motor_hold(motorH1, motorH2);
        reg = potentiometer_Read(analogPin);
        check_direc(reg);
      }
    }
//////////////////////////////////////////////////////
/*                                                  */
/*                       mode 7                     */
/*                                                  */
//////////////////////////////////////////////////////
  if (mode == 7) 
  {
    currentTime = millis();
    mescheck(message);
    reg = potentiometer_Read(analogPin);
    check_direc(reg);

    motor_forward(motorL1, motorL2, 120*start);
    motor_forward(motorR1, motorR2, 120*start); 
    
    if (direc > 0)
    {
      if (direc != direction_state) 
      { 
        if (direc < direction_state){
            motor_forward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
          }
        else if (direc > direction_state){
            motor_backward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
        }
      }
      else
      {
        motor_hold(motorH1, motorH2);
        reg = potentiometer_Read(analogPin);
        check_direc(reg);
      }
    }
    if (currentTime - startTime >= delayTime) mode = 0;
  }

//////////////////////////////////////////////////////
/*                                                  */
/*                       mode 0                     */
/*                                                  */
//////////////////////////////////////////////////////


  else if (mode == 0) 
  {
    
    mescheck(message);
    reg = potentiometer_Read(analogPin);
    check_direc(reg);

    motor_forward(motorL1, motorL2, 110*start);
    motor_forward(motorR1, motorR2, 110*start); 
    
    if (direc > 0)
    {
      if (direc != direction_state) 
      { 
        if (direc < direction_state){
            motor_forward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
          }
        else if (direc > direction_state){
            motor_backward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
        }
      }
      else
      {
        motor_hold(motorH1, motorH2);
        reg = potentiometer_Read(analogPin);
        check_direc(reg);
      }
    }
    check_mode();
    if (distance_front != 2000) {
      if (distance_front < 850) mode = 1;
    }
    else mode = 0;
  }



//////////////////////////////////////////////////////
/*                                                  */
/*                       mode 1                     */
/*                                                  */
//////////////////////////////////////////////////////


  // 왼쪽으로 핸들 꺽고 직진  
  else if (mode == 1)
  {

    // 멈추고
    motor_hold(motorL1,motorL2);
    motor_hold(motorR1,motorR2);
    delay(300);

    // 왼쪽 핸들 돌리기 
    motor_forward(motorH1,motorH2, 100);
    delay(1000);
    motor_hold(motorH1, motorH2);
    delay(100);
    
    // 직진 
    motor_forward(motorL1,motorL2,110);
    motor_forward(motorR1,motorR2,110);
    delay(6000);
    
    mode = 2;
   }

//////////////////////////////////////////////////////
/*                                                  */
/*                       mode 2                     */
/*                                                  */
//////////////////////////////////////////////////////


  else if (mode == 2) 
  {
    motor_forward(motorL1, motorL2, 150);
    motor_forward(motorR1, motorR2, 150);

    mescheck(message);
    reg = potentiometer_Read(analogPin);
    check_direc(reg);
    
    if (direc > 0)
    {
      if (direc != direction_state) 
      { 
        if (direc < direction_state){
            motor_forward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
          }
        else if (direc > direction_state){
            motor_backward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
        }
      }
      else
      {
        motor_hold(motorH1, motorH2);
        reg = potentiometer_Read(analogPin);
        check_direc(reg);
      }
    }
    /*
    distance_side = ultrasonic_distance(sR_trig, sR_echo);
    if (distance_side >= 900) mode = 3;
    else mode = 2; 
    */
  
    check_mode();
    if (distance_front != 2000) {
      if (distance_front < 700) mode = 3;
    }
    else mode = 2;
  }


//////////////////////////////////////////////////////
/*                                                  */
/*                       mode 3                     */
/*                                                  */
//////////////////////////////////////////////////////


  // 오른쪽으로 핸들 꺽고 직진  
  else if (mode == 3)
  {
    // 속도 줄이기 

    motor_hold(motorL1,motorL2);
    motor_hold(motorR1,motorR2);
    delay(300);
    
    // 핸들 돌리기 
    motor_backward(motorH1,motorH2, 200);  //오른쪽
    delay(1000);
    motor_hold(motorH1, motorH2);
    delay(100);
    // 직진 
    motor_forward(motorL1,motorL2,110);
    motor_forward(motorR1,motorR2,110);
    delay(8000);
    
    mode = 4;
   }


//////////////////////////////////////////////////////
/*                                                  */
/*                       mode 4                     */
/*                                                  */
//////////////////////////////////////////////////////


  else if (mode == 4 and cnt < 100) //cnt 증가 있음
  {
    motor_forward(motorL1, motorL2, 155);
    motor_forward(motorR1, motorR2, 155);

    mescheck(message);
    reg = potentiometer_Read(analogPin);
    check_direc(reg);
    
    if (direc > 0)
    {
      if (direc != direction_state) 
      { 
        if (direc < direction_state){
            motor_forward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
          }
        else if (direc > direction_state){
            motor_backward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
        }
      }
      else
      {
        motor_hold(motorH1, motorH2);
        reg = potentiometer_Read(analogPin);
        check_direc(reg);
      }
    }
    cnt ++;
  }

  else if (mode == 4 and cnt == 100) {
    mode = 5;
    red_lock = 1;
  }

//////////////////////////////////////////////////////
/*                                                  */
/*                       mode 5                     */
/*                                                  */
//////////////////////////////////////////////////////

 // 신호등 감지
  else if (mode == 5) 
  {
    motor_forward(motorL1, motorL2, 170*red_lock*start);
    motor_forward(motorR1, motorR2, 170*red_lock*start);
    
    mescheck(message);
    reg = potentiometer_Read(analogPin);
    check_direc(reg);
    
    if (direc > 0)
    {
      if (direc != direction_state) 
      { 
        if (direc < direction_state){
            motor_forward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
          }
        else if (direc > direction_state){
            motor_backward(motorH1, motorH2, 255);
            delay(10);
            motor_hold(motorH1, motorH2);
            reg = potentiometer_Read(analogPin);
            check_direc(reg);
        }
      }
      else
      {
        motor_hold(motorH1, motorH2);
        reg = potentiometer_Read(analogPin);
        check_direc(reg);
      }
    }
  }

//////////////////////////////////////////////////////
/*                                                  */
/*                       mode 6                     */
/*                                                  */
//////////////////////////////////////////////////////

  else if (mode == 6)
  {
    motor_hold(motorL1, motorL2);
    motor_hold(motorR1, motorR2);
  }

}
