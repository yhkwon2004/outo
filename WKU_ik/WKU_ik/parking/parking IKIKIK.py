import rplidar
import serial
import math
import time
import keyboard


# 라이다 및 시리얼 통신 초기화
def initialize_lidar():
    lidar = rplidar.RPLidar('COM3', timeout=3)  # 실제 포트 이름으로 변경 필요
    lidar.stop()
    lidar.stop_motor()
    time.sleep(1)
    lidar.start_motor()
    time.sleep(1)
    return lidar

# 아두이노 시리얼 통신 초기화
arduino = serial.Serial('COM4', 9600, timeout=1)  # 실제 포트 이름으로 변경 필요

# 상수 정의
SEARCH_MIN_DIST = 750   # 75cm
SEARCH_MAX_DIST = 1200  # 120cm
PARK_MIN_DIST = 100     # 10cm
PARK_MAX_DIST = 1400    # 140cm
MIN_SPACE_WIDTH = 950  # 최소 주차 공간 너비 (mm)
MAX_SPACE_WIDTH = 1000  # 최대 주차 공간 너비 (mm)
CAR_WIDTH = 500  # 차량 폭 (mm)
MAX_STEERING_ANGLE = 20  # 최대 조향 각도

def normalize_angle(angle):
    return (angle + 360) % 360

def print_lidar_data(scan):
    print("라이다 데이터:")
    for data in scan:
        if len(data) == 3:  # (quality, angle, distance) 형식
            quality, angle, distance = data
        elif len(data) == 2:  # (angle, distance) 형식
            angle, distance = data
            quality = "N/A"
        else:
            continue
        
        angle = normalize_angle(angle)
        print(f"각도: {angle:.2f}°, 거리: {distance:.2f}mm, 품질: {quality}")
    print("------------------------")

def detect_parking_space(scan):
    ANGLE_RANGE = (270,330)
    MIN_DIST = SEARCH_MIN_DIST
    MAX_DIST = SEARCH_MAX_DIST
    
    print_lidar_data(scan)  # 라이다 데이터 출력
    
    filtered_scan = []
    for data in scan:
        if len(data) == 3:  # (quality, angle, distance) 형식
            _, angle, distance = data
        elif len(data) == 2:  # (angle, distance) 형식
            angle, distance = data
        else:
            continue  # 예상치 못한 데이터 형식은 무시

        angle = normalize_angle(angle)
        if ANGLE_RANGE[0] <= angle <= ANGLE_RANGE[1] and MIN_DIST <= distance <= MAX_DIST:
            filtered_scan.append((angle, distance))
    
    filtered_scan.sort(key=lambda x: x[0])  # 각도로 정렬
    
    prev_distance = None
    gap_start = None
    gap_max_distance = 0

    for angle, distance in filtered_scan:
        if prev_distance is None:
            prev_distance = distance
            continue

        if abs(distance - prev_distance) > 500:  # 50cm 이상 차이나면 갭 시작
            if gap_start is None:
                gap_start = (angle, prev_distance)
                gap_max_distance = distance
            elif distance < prev_distance and abs(distance - gap_start[1]) < 100:  # 갭 종료, 10cm 이내
                gap_end = (angle, distance)
                gap_width = calculate_arc_length(gap_start[0], gap_end[0], (gap_start[1] + gap_end[1]) / 2)
                if MIN_SPACE_WIDTH <= gap_width <= MAX_SPACE_WIDTH:
                    return {
                        'start': gap_start[0],
                        'end': gap_end[0],
                        'width': gap_width,
                        'max_distance': gap_max_distance
                    }
                gap_start = None
                gap_max_distance = 0
        elif gap_start is not None:
            gap_max_distance = max(gap_max_distance, distance)

        prev_distance = distance

    return None

def calculate_arc_length(start_angle, end_angle, radius):
    angle_diff = normalize_angle(end_angle - start_angle)
    return (angle_diff / 360) * 2 * math.pi * radius

def calculate_steering_angle(scan, parking_space):
    start_angle = parking_space['start']
    end_angle = parking_space['end']
    
    start_dist = min([dist for angle, dist in scan if abs(normalize_angle(angle - start_angle)) < 5])
    end_dist = min([dist for angle, dist in scan if abs(normalize_angle(angle - end_angle)) < 5])
    
    center_angle = normalize_angle((start_angle + end_angle) / 2)
    center_dist = (start_dist + end_dist) / 2
    
    vehicle_direction = 0
    
    angle_difference = normalize_angle(center_angle - vehicle_direction)
    if angle_difference > 180:
        angle_difference -= 360
    
    distance_factor = 1 - (center_dist / PARK_MAX_DIST)
    adjusted_angle = angle_difference * distance_factor
    
    steering_angle = max(-MAX_STEERING_ANGLE, min(MAX_STEERING_ANGLE, adjusted_angle))
    
    return steering_angle

def control_steering_for_parking(scan, parking_space):
    steering_angle = calculate_steering_angle(scan, parking_space)
    return map_angle_to_command(steering_angle)

def check_parking_complete(scan):
    rear_distance = min([dist for angle, dist in scan if 330 <= angle or angle <= 30])
    return rear_distance < 300  # 30cm 이내면 주차 완료

def map_angle_to_command(angle):
    mapped = int(((angle + MAX_STEERING_ANGLE) / (2 * MAX_STEERING_ANGLE)) * 20) + 1
    return chr(ord('a') + mapped - 1)  # 'a'부터 'u'까지의 문자로 변환

def send_to_arduino(command):
    arduino.write(command.encode())
    time.sleep(0.1)

def main():
    global lidar
    try:
        lidar = initialize_lidar()
        
        print("출발")
        send_to_arduino("2")  # 전진 명령
        time.sleep(2)  # 2초 대기
        
        print("주차 공간 탐색 시작")
        parking_space = None
        
        for scan in lidar.iter_scans():
            if keyboard.is_pressed('q'):
                print("'Q' 키가 눌렸습니다. 프로그램을 종료합니다.")
                break
            
            parking_space = detect_parking_space(scan)
            if parking_space:
                print("주차 공간 감지됨")
                print(f"주차 공간 정보: 시작 각도 {parking_space['start']:.2f}°, 끝 각도 {parking_space['end']:.2f}°, 너비 {parking_space['width']:.2f}mm")
                send_to_arduino("0")  # 정지 명령
                time.sleep(1)  # 완전히 정지할 때까지 대기
                break
        
        if parking_space:
            print("주차 시작")
            while not check_parking_complete(scan):
                if keyboard.is_pressed('q'):
                    print("'Q' 키가 눌렸습니다. 프로그램을 종료합니다.")
                    break
                
                steering_command = control_steering_for_parking(scan, parking_space)
                print(f"조향 명령: {steering_command}")
                send_to_arduino(steering_command)
                send_to_arduino("3")  # 후진 명령
                time.sleep(0.5)
                send_to_arduino("f")  # 조향 중립
                
                scan = next(lidar.iter_scans())  # 새로운 스캔 데이터 획득
            
            send_to_arduino("0")  # 정지
            print("주차 완료")
        else:
            print("주차 공간을 찾지 못했습니다.")

    except rplidar.RPLidarException as e:
        print(f"라이다 예외 발생: {e}")
    except KeyboardInterrupt:
        print("프로그램 종료")
    finally:
        lidar.stop()
        lidar.stop_motor()
        lidar.disconnect()
        arduino.close()

if __name__ == '__main__':
    main()