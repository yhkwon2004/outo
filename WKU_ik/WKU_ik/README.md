## 1) 프로젝트 H/W 구성
유아용 전동차를 개조하여 차량 구성
### - 조향
: 조향모터를 사용하지 않고, 기존 차량의 핸들 부분에 DC모터를 부착하여 핸들을 돌리는 힘을 주는 방식 사용


: 자율주행을 위해서는 실시간으로 핸들이 돌아간 정도를 인식할 수 있어야 함. 이를 위하여 핸들 부분에 가변저항을 부착, 아두이노를 통해 가변저항의 값을 받아옴. 가변저항 값을 인식함으로써 현재 차량 앞바퀴의 방향성을 확인 가능. 
### - 구동
: 기존 차량의 바퀴에 DC모터 부착하여 사용
### - 사용한 센서
**[카메라]**
: 주차 미션을 제외한 모든 미션 수행에 사용


**[RPLidar A2]**
: 360도 2D 공간 스캔 가능, 주차 미션에 사용


**[초음파센서]**
: 회피기동 구현에 사용 

## 2) 파일 설명
수정 사항에 대비하여 파일 이름 모두 영어로 작성함. 변경하지 말 것.
<AUTODRIVING>: 자율주행 관련 폴더
<MISSION>: 회피기동 및 신호등 미션 관련 폴더 
<PARKING>: 주차미션 관련 폴더 

