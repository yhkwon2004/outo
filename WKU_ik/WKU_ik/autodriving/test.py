import cv2

def check_camera(index):
    cap = cv2.VideoCapture(index)
    if not cap.isOpened():
        print(f"카메라 인덱스 {index}를 열 수 없습니다.")
        return False
    ret, frame = cap.read()
    cap.release()
    if ret:
        print(f"카메라 인덱스 {index}가 사용 가능합니다.")
        return True
    else:
        print(f"카메라 인덱스 {index}에서 프레임을 읽을 수 없습니다.")
        return False

# 여러 카메라 인덱스를 확인
for i in range(10):  # 0부터 9까지의 인덱스 확인
    if check_camera(i):
        print(f"사용 가능한 카메라를 찾았습니다: 인덱스 {i}")
    else:
        print(f"인덱스 {i}에서 사용 가능한 카메라를 찾지 못했습니다.")

print("카메라 확인이 완료되었습니다.")