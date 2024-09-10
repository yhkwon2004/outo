import Function_Library as fl
import time
import math

# Initial Setting
if (__name__ == "__main__"):
    arduino_port = 'COM6'
    ser = fl.libARDUINO()
    comm = ser.init(arduino_port, 9600)
    serial_comm = 'H'
    comm.write(serial_comm.encode())

    env = fl.libLIDAR('COM5')
    env.init()
    env.getState()
    env.setRPM(600)

    mode = 'PARKING1'
    scan_able = 1
    scan_count = 0
    front_distance = [0]
    front_angle = [0]
    back_left_distance = [0]
    back_left_angle = [0]
    back_right_distance = [0]
    back_right_angle = [0]
    front_idx, back_left_idx, back_right_idx = [0, 0, 0]
    front_first_idx, left_first_idx, right_first_idx = [0, 0, 0]
    front_count, back_left_count, back_right_count = [0, 0, 0]

    # 주차
    car_check = 0   # 감지된 차량 수
    car1_count = 0
    car2_count = 0
    parking_count = 0   # 주차 단계 count
    d1 = 0
    d2 = 0
    a1 = 0
    a2 = 0
    a_cond = 0
    d_cal = 0
    a_cal = 0
    a_temp = 0
    a_temp_list = []
    a_cond_list = []

start_comm = input('start : ')
if start_comm == '0':
    comm.write(start_comm.encode())

while True:
    if scan_able == 1:
        # LIDAR scan loop
        for scan in env.scanning():
            scan_count += 1
            # 각각의 data들이 불규칙한 다중 리스트 형식으로 저장 [[각도, 거리],[각도, 거리],...]
            # 후방 각도 50미만, 310초과 불가
            front_temp = env.getAngleDistanceRange(scan, 180, 220, 100, 2500)
            back_left_temp = env.getAngleDistanceRange(scan, 110, 120, 100, 200)
            back_right_temp = env.getAngleDistanceRange(scan, 240, 290, 100, 1500)

            if len(front_temp[:, 1:]) != 0:
                front_distance.extend(front_temp[:, 1])
                front_angle.extend(front_temp[:, 0])
                front_idx = len(front_distance) - 1
                front_count = scan_count
            if len(back_left_temp[:, 1:]) != 0:
                back_left_distance.extend(back_left_temp[:, 1])
                back_left_angle.extend(back_left_temp[:, 0])
                back_left_idx = len(back_left_distance) - 1
                back_left_count = scan_count
            if len(back_right_temp[:, 1:]) != 0:
                back_right_distance.extend(back_right_temp[:, 1])
                back_right_angle.extend(back_right_temp[:, 0])
                back_right_idx = len(back_right_distance) - 1
                back_right_count = scan_count

            if mode == 'PARKING1':
                d1 = back_right_distance[back_right_idx]
                a1 = back_right_angle[back_right_idx]

                if car_check == 0 and 269 < back_right_angle[back_right_idx] < 271:
                    car_check = 1
                    car1_count = scan_count
                    d2 = front_distance[front_idx]
                    a2 = front_angle[front_idx]

                if car_check == 1 and d1 != 0 and a1 != 0 and d2 != 0 and a2 != 0:
                    d_cal = math.sqrt(d1**2 + d2**2 - 2*d1*d2*math.cos(math.radians(a1-a2)))
                    a_temp = math.degrees(math.asin(d1 * math.sin(math.radians(a1 - a2)) / d_cal))
                    a_cond = 180 - a_temp - (a1 - a2)
                    a_temp_list.append(a_temp)
                    a_cond_list.append(a_cond)

                # 0.5초 경과 후 보정1 / 1.2초 경과 후 보정2
                if car_check == 1:
                    if scan_count - car1_count == 5:
                        serial_comm = 'T'
                        comm.write(serial_comm.encode())
                        if a_temp_list[0] > 50:
                            serial_comm = '-25'
                        elif 35 < a_temp_list[0] <= 50:
                            serial_comm = str(-int(round(a_temp_list[0] - 26)))
                        elif 30 < a_temp_list[0] <= 35:
                            serial_comm = str(-int(round(a_temp_list[0] - 20)))
                        elif 26 < a_temp_list[0] <= 30:
                            serial_comm = str(-int(round(3*a_temp_list[0]/2 - 34)))
                        elif 22 < a_temp_list[0] <= 26:
                            serial_comm = str(int(round(a_temp_list[0]/5)))
                        elif 19 < a_temp_list[0] <= 22:
                            serial_comm = str(int(round(-5*a_temp_list[0]/3 + 125/3)))
                        elif 10 <= a_temp_list[0] <= 19:
                            serial_comm = str(int(round(-13*a_temp_list[0]/9 + 38.4)))
                        elif a_temp_list[0] < 10:
                            serial_comm = '25'
                        angle = int(serial_comm)
                        comm.write(serial_comm.encode())
                        print(serial_comm)
                    if scan_count - car1_count == 12:
                        print('temp ', a_temp_list)
                        serial_comm = 'T'
                        comm.write(serial_comm.encode())
                        if 15 < abs(angle) <= 25:
                            if angle < 0:
                                serial_comm = str(int(round(3 * angle / 5 + 4)))
                            else:
                                serial_comm = str(int(round(3 * angle / 5 - 3)))
                        elif 6 <= abs(angle) <= 15:
                            serial_comm = str(int(round(4*angle/5)))
                        elif abs(angle) < 6:
                            serial_comm = str(-angle)
                        comm.write(serial_comm.encode())
                        check = int(serial_comm)
                        print(serial_comm)

                # LIDAR off
                if car_check == 1 and back_right_angle[back_right_idx] > 273 and scan_count - car1_count > 95:
                    car_check = 2
                    car2_count = scan_count
                    serial_comm = 'H'
                    comm.write(serial_comm.encode())
                    env.stop()
                    scan_able = 0
                    break

    if mode == 'PARKING1':
        parking_count += 1
        for i in range(1, 9):
            if parking_count == i:
                if i == 5 and check > 0:
                    serial_comm = '9'
                    comm.write(serial_comm.encode())
                    break
                serial_comm = str(i)
                comm.write(serial_comm.encode())
                break
        time.sleep(6)
