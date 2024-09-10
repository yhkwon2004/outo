#include <Car_Library.h>
#include <string.h>

int analogPin = A0; //가변저항 output pin

int motorH1 = 11;
int motorH2 = 10;
int motorL1 = 8;
int motorL2 = 9;
int motorR1 = 13;
int motorR2 = 12;

char message = 'M';
int waitForAngle = 0;

int reg=0;
int count = 0;
int angle = 0; 
int speed = 0; 

void move(char fowardback, int delay_ms, int speed)
{
  switch (fowardback){
    case 'F':
      motor_forward(motorL1,motorL2,speed);
      motor_forward(motorR1,motorR2,speed);
      delay(delay_ms);
      break;
    case 'B':
      motor_backward(motorL1,motorL2,speed);
      motor_backward(motorR1,motorR2,speed);
      delay(delay_ms);
      break;
    case 'H':
      delay(delay_ms);
      break;
  }
  motor_hold(motorL1, motorL2);
  motor_hold(motorR1, motorR2);
  delay(20);
}

void handle(char leftright, int delay_ms)
{
  motor_hold(motorH1, motorH2);
  delay(20);
  switch (leftright){
      case 'L':
        motor_forward(motorH1,motorH2,150);
        break;
      case 'R':
        motor_backward(motorH1,motorH2,150);
        break;
  }
  delay(delay_ms);
  motor_hold(motorH1, motorH2);
}

void move2(char direction, int delay_ms, int speed, int angle)
{
  int ang2reg = angle2reg(angle);
  //모터 속도 비율 확인
  handle2(142);
  //FL FC FR BL BC BR
  switch (direction){
    case 'A':
      motor_forward(motorL1,motorL2,speed);
      motor_forward(motorR1,motorR2,speed);
      delay(delay_ms/4);
      handle2(ang2reg);
      motor_forward(motorL1,motorL2,speed/2);
      motor_forward(motorR1,motorR2,speed);
      delay(delay_ms/2);
      handle2(142);
      motor_forward(motorL1,motorL2,speed);
      motor_forward(motorR1,motorR2,speed);
      delay(delay_ms/4);
      break;
    case 'B':
      motor_forward(motorL1,motorL2,speed);
      motor_forward(motorR1,motorR2,speed);
      delay(delay_ms);
      break;
    case 'C':
      motor_forward(motorL1,motorL2,speed);
      motor_forward(motorR1,motorR2,speed);
      delay(delay_ms/4);
      handle2(ang2reg);
      motor_forward(motorL1,motorL2,speed);
      motor_forward(motorR1,motorR2,speed/2);
      delay(delay_ms/2);
      motor_forward(motorL1,motorL2,speed);
      motor_forward(motorR1,motorR2,speed);
      delay(delay_ms/4);
      break;
    case 'D':
      motor_backward(motorL1,motorL2,speed);
      motor_backward(motorR1,motorR2,speed);
      delay(delay_ms/4);
      handle2(ang2reg);
      motor_backward(motorL1,motorL2,speed/2);
      motor_backward(motorR1,motorR2,speed);
      delay(delay_ms/2);
      motor_backward(motorL1,motorL2,speed);
      motor_backward(motorR1,motorR2,speed);
      delay(delay_ms/4);
      break;
    case 'E':
      motor_backward(motorL1,motorL2,speed);
      motor_backward(motorR1,motorR2,speed);
      delay(delay_ms);
      break;
    case 'F':
      motor_backward(motorL1,motorL2,speed);
      motor_backward(motorR1,motorR2,speed);
      delay(delay_ms/4);
      handle2(ang2reg);
      motor_backward(motorL1,motorL2,speed);
      motor_backward(motorR1,motorR2,speed/2);
      delay(delay_ms/2);
      motor_backward(motorL1,motorL2,speed);
      motor_backward(motorR1,motorR2,speed);
      delay(delay_ms/4);
      break;
    case 'H':
      break;
  }
  motor_hold(motorL1, motorL2);
  motor_hold(motorR1, motorR2);
  handle2(142);
}

void handle2(int target)
{
  reg = potentiometer_Read(analogPin);
  while (reg > target)
  {
    motor_backward(motorH1,motorH2,150);
    delay(30);
    reg = potentiometer_Read(analogPin);
  }
  while (reg < target)
  {
    motor_forward(motorH1,motorH2,150);
    delay(30);
    reg = potentiometer_Read(analogPin);
  }
  motor_hold(motorH1,motorH2);
}

int angle2reg(int angle)
{
  if (angle > 24) return 130;
  else if (2 < angle <= 24) return 142 - angle*12/22;
  else if (-2 <= angle <= 2) return 142;
  else if (-24 <= angle < -2) return 142 - angle*15/22;
  else if (angle < -24)  return 159; 
}

void path_modify(int angle)
{
  int reg_old = angle2reg(angle);

  if (angle > 24)
  {
    handle2(159);
    delay(2000);
    handle2(135);
    delay(1000);
    handle2(142);
    delay(500);
  }
  else if (2 < angle <= 24)
  {
    handle2(142 + 142 - reg_old);
    delay(2000);
    handle2(139);
    delay(1500);
  }
  else if (-24 <= angle < -2)
  {
    handle2(144 + 142 - reg_old);
    delay(1800);
    handle2(160);
    delay(1700);
  }
  else if (angle < -24)
  {
    handle2(130);
    delay(2000);
    handle2(159);
    delay(1000);
    handle2(142);
    delay(500);
  }
  else if (-2 <= angle <= 2)
  {
    delay(3000);
  }
  handle2(142);
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(motorH1, OUTPUT);
  pinMode(motorH2, OUTPUT);
  pinMode(motorL1, OUTPUT);
  pinMode(motorL2, OUTPUT);
  pinMode(motorR1, OUTPUT);
  pinMode(motorR2, OUTPUT);
}

void loop()
{  
  if (Serial.available()) 
  {
    if (waitForAngle==1) 
    {
      angle = Serial.parseInt();
      waitForAngle = 0;
    } 
    else
    {
      message = Serial.read();
      count = 0;
      if (message == 'T') waitForAngle = 1;    
    }
  }

  if (angle != 0)
  {
    path_modify(angle);
    angle = 0;
  }

  if (count == 0)
  {
    switch (message){
      case '0':
        handle2(142);
        delay(3000);
        motor_forward(motorL1, motorL2, 50);
        motor_forward(motorR1, motorR2, 50);
        count=1;
        break;
      case '1':
        move('H',1000,0);
        handle2(160);
        move('F', 5500, 50);
        count=1;
        break;
      case '2':
        move('H',20,0);
        handle2(129);
        move('B',8000,50);
        count=1;
        break;
      case '3':
        move('H',20,0);
        handle2(158);
        move('F',3000,50);
        count=1;
        break;
      case '4':
        move('H',20,0);
        handle2(130);
        motor_backward(motorL1, motorL2, 40);
        motor_backward(motorR1, motorR2, 25);
        delay(10000);
        move('H',20,0);
        handle2(150);
        move('F',5000,30);
        count=1;
        break;
      case '5':
        move('H',20,0);
        handle2(135);
        move('B',4500,50);
        handle2(142);
        move('B',6300,30);
        move('H',5000,0);
        count=1;
        break;
      case '6':
        move('F',3000,70);
        count=1;
        break;
      case '7':
        handle2(159);
        motor_forward(motorL1,motorL2,20);
        motor_forward(motorR1,motorR2,150);
        delay(6000);
        handle2(142);
        move('F',9000,100);
        count=1;
        break;
      case '8':
        move('H',0,0);
        count=1;
        break;  
      case '9':
        move('H',20,0);
        handle2(135);
        move('B',4500,50);
        handle2(142);
        move('B',4500,30);
        move('H',5000,0);
        count=1;
        break;     
      case 'S':
        move('H',0,0);
        delay(3000);
        motor_forward(motorH1,motorH2,0);
        motor_forward(motorR1,motorR2,250);
        motor_backward(motorL1,motorL2,250);
        delay(3500);
        move('H',0,0);
        handle2(138);
        delay(1000);
        move('B',8000,50);
        handle2(142);
        move('B',4800,30);
        count=1;
        break;
      case 'T': 
        count=1;
        break;       
      case 'H':
        move('H',1000,0);
        count=1;
        break;
      case 'M':
        handle2(142);
        //motor_forward(motorL1,motorL2,100);
        //motor_forward(motorR1,motorR2,100);
        count=1;
        break;     
      default:
        move('H',0,0);
        count=1;
        break;
    }
  }
}