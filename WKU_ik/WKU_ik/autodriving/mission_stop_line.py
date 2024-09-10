# This Python file uses the following encoding: utf-8
# -*- coding: cp949 -*-
# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import matplotlib.image as mpimg
import numpy as np
import cv2
import random
import Function_Library as fl
import serial

global prediction_buff
prediction_buff = 5
stop_sign = 0
FORWARD_THRESHOLD = 0.3
LEFT4, LEFT3, LEFT2, LEFT1, FORWARD, RIGHT1, RIGHT2, RIGHT3, RIGHT4, = (1, 2, 3, 4, 5, 6, 7, 8, 9)
DIRECTION = ("a", "LEFT4", "LEFT3", "LEFT2", "LEFT1", "FORWARD", "RIGHT1", "RIGHT2", "RIGHT3", "RIGHT4")

# %matplotlib inline

# cap = cv2.VideoCapture('solidWhiteRight.mp4')
# cap = cv2.VideoCapture('lane.mp4')
# cap = cv2.VideoCapture(1)
fit_result, l_fit_result, r_fit_result, L_lane, R_lane = [], [], [], [], []
S_lane = []

arduino_port = 'COM4'
ser = fl.libARDUINO()
comm = ser.init(arduino_port, 9600)
d1 = 13
direc = ' '
stop = 0
stop1 = 0

# Define the codec and create VideoWriter object
# fourcc = cv2.VideoWriter_fourcc(*'mp4v') # Be sure to use the lower case
# out = cv2.VideoWriter('output.mp4', fourcc, 20.0, ( 960, 540 ))

def grayscale(img):
    """Applies the Grayscale transform
    This will return an image with only one color channel
    but NOTE: to see the returned image as grayscale
    you should call plt.imshow(gray, cmap='gray')"""
    return cv2.cvtColor(img, cv2.COLOR_RGB2GRAY)


def canny(img, low_threshold, high_threshold):
    """Applies the Canny transform"""
    return cv2.Canny(img, low_threshold, high_threshold)


def gaussian_blur(img, kernel_size):
    """Applies a Gaussian Noise kernel"""
    return cv2.GaussianBlur(img, (kernel_size, kernel_size), 0)


def region_of_interest(img, vertices):
    """
    Applies an image mask.

    Only keeps the region of the image defined by the polygon
    formed from `vertices`. The rest of the image is set to black.
    """
    # defining a blank mask to start with
    mask = np.zeros_like(img)

    # defining a 3 channel or 1 channel color to fill the mask with depending on the input image
    if len(img.shape) > 2:
        channel_count = img.shape[2]  # i.e. 3 or 4 depending on your image
        ignore_mask_color = (255,) * channel_count
    else:
        ignore_mask_color = 255

    # filling pixels inside the polygon defined by "vertices" with the fill color
    cv2.fillPoly(mask, vertices, ignore_mask_color)

    # returning the image only where mask pixels are nonzero
    masked_image = cv2.bitwise_and(img, mask)
    return masked_image


def draw_lines(img, lines, color=[255, 0, 0], thickness=2):
    """
    NOTE: this is the function you might want to use as a starting point once you want to
    average/extrapolate the line segments you detect to map out the full
    extent of the lane (going from the result shown in raw-lines-example.mp4
    to that shown in P1_example.mp4).

    Think about things like separating line segments by their
    slope ((y2-y1)/(x2-x1)) to decide which segments are part of the left
    line vs. the right line.  Then, you can average the position of each of
    the lines and extrapolate to the top and bottom of the lane.

    This function draws `lines` with `color` and `thickness`.
    Lines are drawn on the image inplace (mutates the image).
    If you want to make the lines semi-transparent, think about combining
    this function with the weighted_img() function below
    """
    for line in lines:
        for x1, y1, x2, y2 in line:
            cv2.line(img, (x1, y1), (x2, y2), color, thickness)


def draw_circle(img, lines, color=[0, 0, 255]):
    for line in lines:
        cv2.circle(img, (line[0], line[1]), 2, color, -1)


def hough_lines(img, rho, theta, threshold, min_line_len, max_line_gap):
    """
    `img` should be the output of a Canny transform.
    Returns an image with hough lines drawn.
    """
    lines = cv2.HoughLinesP(img, rho, theta, threshold, np.array([]), minLineLength=min_line_len,
                            maxLineGap=max_line_gap)
    line_arr = np.zeros((img.shape[0], img.shape[1], 3), dtype=np.uint8)
    # draw_lines(line_arr, lines)
    return lines


def weighted_img(img, initial_img, α=0.8, β=1., λ=0.):
    """
    `img` is the output of the hough_lines(), An image with lines drawn on it.
    Should be a blank image (all black) with lines drawn on it.

    `initial_img` should be the image before any processing.

    The result image is computed as follows:

    initial_img * α + img * β + λ
    NOTE: initial_img and img must be the same shape!
    """
    return cv2.addWeighted(initial_img, α, img, β, λ)


def Collect_points(lines):
    # reshape [:4] to [:2]
    interp = lines.reshape(lines.shape[0] * 2, 2)
    # interpolation & collecting points for RANSAC
    for line in lines:
        if np.abs(line[3] - line[1]) > 5:
            tmp = np.abs(line[3] - line[1])
            a = line[0];
            b = line[1];
            c = line[2];
            d = line[3]
            slope = (line[2] - line[0]) / (line[3] - line[1])
            for m in range(0, tmp, 5):
                if slope > 0:
                    new_point = np.array([[int(a + m * slope), int(b + m)]])
                    interp = np.concatenate((interp, new_point), axis=0)
                elif slope < 0:
                    new_point = np.array([[int(a - m * slope), int(b - m)]])
                    interp = np.concatenate((interp, new_point), axis=0)
    return interp


def get_random_samples(lines):
    one = random.choice(lines)
    two = random.choice(lines)
    if (two[0] == one[0]):  # extract again if values are overlapped
        while two[0] == one[0]:
            two = random.choice(lines)
    one, two = one.reshape(1, 2), two.reshape(1, 2)
    three = np.concatenate((one, two), axis=1)
    three = three.squeeze()
    return three


def compute_model_parameter(line):
    # y = mx+n
    m = (line[3] - line[1]) / (line[2] - line[0])
    n = line[1] - m * line[0]
    # ax+by+c = 0
    a, b, c = m, -1, n
    par = np.array([a, b, c])
    return par


def compute_distance(par, point):
    # distance between line & point

    return np.abs(par[0] * point[:, 0] + par[1] * point[:, 1] + par[2]) / np.sqrt(par[0] ** 2 + par[1] ** 2)


def model_verification(par, lines):
    # calculate distance
    distance = compute_distance(par, lines)
    # total sum of distance between random line and sample points
    sum_dist = distance.sum(axis=0)
    # average
    avg_dist = sum_dist / len(lines)

    return avg_dist


def draw_extrapolate_line(img, par, color=(0, 0, 255), thickness=2):
    x1, y1 = int(-par[1] / par[0] * img.shape[0] - par[2] / par[0]), int(img.shape[0])
    x2, y2 = int(-par[1] / par[0] * (img.shape[0] / 2 + 100) - par[2] / par[0]), int(img.shape[0] / 2 + 100)
    cv2.line(img, (x1, y1), (x2, y2), color, thickness)
    return img


def get_fitline(img, f_lines):
    rows, cols = img.shape[:2]
    output = cv2.fitLine(f_lines, cv2.DIST_L2, 0, 0.01, 0.01)
    vx, vy, x, y = output[0], output[1], output[2], output[3]
    x1 = int(((img.shape[0] - 1) - y[0]) / (vy[0] + 0.0001) * vx[0] + x[0])
    y1 = img.shape[0] - 1
    x2 = int(((img.shape[0] / 2 + 100) - y[0]) / (vy[0] + 0.0001) * vx[0] + x[0])
    y2 = int(img.shape[0] / 2 + 100)
    result = [x1, y1, x2, y2]

    return result



def draw_stop_line(img, result, color=(346,96,93), thickness=10):
    global stop
    lane = np.zeros_like(img)
    cv2.line(lane, (int(result[0]), int(result[1])), (int(result[2]), int(result[3])), color, thickness)
    final_stop = weighted_img(lane, img, 1, 0.5)

    xa, ya, xb, yb = int(result[0]), int(result[1]), int(result[2]), int(result[3])

    if xb - xa == 0:
        grad = 0
    else:
        grad = (yb - ya) / -(xb - xa)  # the third quadrant

    y_int = ya - grad * xa

    if np.abs(grad) < 0.2 and 370 < grad*240 + y_int < 390:
        stop = 1

    cv2.imshow('stop', final_stop)
    # print(np.abs(grad))
    # print(grad*240 + y_int)

    return stop


def draw_fitline(img, result_l, result_r, color=(255, 0, 255), thickness=10):
    prediction = FORWARD
    global prediction_buff
    # draw fitting line
    lane = np.zeros_like(img)
    cv2.line(lane, (int(result_l[0]), int(result_l[1])), (int(result_l[2]), int(result_l[3])), color, thickness)
    cv2.line(lane, (int(result_r[0]), int(result_r[1])), (int(result_r[2]), int(result_r[3])), color, thickness)
    cv2.line(lane, (int((result_l[0] + result_r[0]) / 2), int((result_l[1] + result_r[1]) / 2)),
             (int((result_l[2] + result_r[2]) / 2), int((result_l[3] + result_r[3]) / 2)), color, thickness)

    xa, ya, xb, yb = int((result_l[0] + result_r[0]) / 2), int((result_l[1] + result_r[1]) / 2), \
                     int((result_l[2] + result_r[2]) / 2), int((result_l[3] + result_r[3]) / 2)

    if yb - ya == 0:
        grad = 0
    else:
        grad = (xb - xa) / -(yb - ya)  # the third quadrant

    turn = 125 #작을수록 민감
    out_turn = 200
    ft = 575

    if int(result_l[0]) < 280 and int(result_r[0]) > 200 \
            and np.abs(result_l[2] - result_r[2]) < 150 or np.abs(grad) > 4:
        prediction = prediction_buff
    elif int(result_l[0]) > out_turn and xa < 480 - turn and grad < 0:
        prediction = LEFT4
    elif int(result_l[0]) > out_turn and 480 - turn < xa <= 480 - turn + 30:
        prediction = LEFT3
    elif int(result_l[0]) > out_turn and 480 - turn + 30 < xa <= 480 - turn + 60:
        prediction = LEFT2
    elif int(result_l[0]) > out_turn and xa >= 480 - turn + 60:
        prediction = FORWARD
    elif int(result_r[0]) < 480 - out_turn and xa > turn-30 and grad > 0:
        prediction = RIGHT4
    elif int(result_r[0]) < 480 - out_turn and turn - 50 < xa <= turn-30:
        prediction = RIGHT3
    elif int(result_r[0]) < 480 - out_turn and turn - 60 < xa <= turn - 50:
        prediction = RIGHT2
    elif int(result_r[0]) < 480 - out_turn and xa <= turn - 60:
        prediction = FORWARD
    elif ft-5 < int(result_r[0]+result_r[2]/2) < ft+5:
        prediction = FORWARD
    elif ft-50 < int(result_r[0]+result_r[2]/2) < ft-5:
        prediction = LEFT1
    elif int(result_r[0]+result_r[2]/2) < ft-50:
        prediction = LEFT2
    elif ft+5 < int(result_r[0]+result_r[2]/2) < ft + 50:
        prediction = RIGHT1
    elif ft + 50 < int(result_r[0] + result_r[2] / 2):
        prediction = RIGHT2
    # print(result_r[0]+result_r[2]/2)
    prediction_buff = prediction
    # add original image & extracted lane lines
    final = weighted_img(lane, img, 1, 0.5)

    return final, prediction


def erase_outliers(par, lines):
    # distance between best line and sample points
    distance = compute_distance(par, lines)

    # filtered_dist = distance[distance<15]
    filtered_lines = lines[distance < 13, :]
    return filtered_lines


def smoothing(lines, pre_frame):
    # collect frames & print average line
    lines = np.squeeze(lines)
    avg_line = np.array([0, 0, 0, 0])

    for ii, line in enumerate(reversed(lines)):
        if ii == pre_frame:
            break
        avg_line += line
    avg_line = avg_line / pre_frame

    return avg_line


def ransac_line_fitting(img, lines, min=100):
    global fit_result, l_fit_result, r_fit_result
    best_line = np.array([0, 0, 0])
    if len(fit_result) == 0:
        fit_result = [0, 0, 0, 0]
    if fit_result[2] - fit_result[0] != 0:
        frv = (fit_result[3] - fit_result[1]) / (fit_result[2] - fit_result[0])
    else:
        frv = 1

    if (len(lines) != 0):
        for i in range(30):
            sample = get_random_samples(lines)
            parameter = compute_model_parameter(sample)
            cost = model_verification(parameter, lines)
            if cost < min:  # update best_line
                min = cost
                best_line = parameter
            if min < 3: break
        # erase outliers based on best line
        filtered_lines = erase_outliers(best_line, lines)
        fit_result = get_fitline(img, filtered_lines)
    else:
        if frv < 0:
            l_fit_result = fit_result
            return l_fit_result
        else:
            r_fit_result = fit_result
            return r_fit_result

    if frv < 0:
        l_fit_result = fit_result
        return l_fit_result
    else:
        r_fit_result = fit_result
        return r_fit_result

def filter_colors(img):
    # HSV로 색 추출
    low = 50
    high = 255
    hsvLower = np.array([low, low, 0])  # 추출할 색의 하한(HSV)
    hsvUpper = np.array([high, high, 255])  # 추출할 색의 상한(HSV)
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)  # 이미지를 HSV으로 변환
    hsv_mask = cv2.inRange(hsv, hsvLower, hsvUpper)  # HSV에서 마스크를 작성
    result = cv2.bitwise_and(img, img, mask=hsv_mask)  # 원래 이미지와 마스크를 합성

    return result

def detect_stop_line(img):
    global stop, stop_sign
    # Set ROI
    vertices = np.array(
        [[(0, 360), (140, 190), (480 - 140, 190), (480 - 0, 360),
          (480 - 100, 360), (300, 340), (180, 340), (100, 360)]],
        dtype=np.int32)
    ROI_img = region_of_interest(img, vertices)
    cv2.imshow('mask', ROI_img)

    # Convert to grayimage
    g_img = grayscale(ROI_img)

    # Apply gaussian filter
    blur_img = gaussian_blur(g_img, 7)  # 7

    # Apply Canny edge transform
    canny_img = canny(blur_img, 200, 300)  # 200, 300
    # to except contours of ROI image

    # if prediction_buff == FORWARD & LEFT1 & RIGHT1:
    vertices2 = np.array(
        [[(0 + 3, 360 - 2), (140 + 2, 190 + 2), (480 - 140 - 2, 190 + 2), (480 - 0 - 3, 360 - 2),
          (480 - 100 + 2, 360 - 2),
          (300 + 2, 340 - 2), (180 - 2, 340 - 2),
          (100 - 2, 360 - 2)]],
        dtype=np.int32)

    canny_img = region_of_interest(canny_img, vertices2)
    # cv2.imshow('canny', canny_img)

    # Perform hough transform
    # Get first candidates for real lane lines
    line_arr = hough_lines(canny_img, 1, 1 * np.pi / 180, 30, 10, 20)

    # draw_lines(img, line_arr, thickness=2)

    if line_arr is None:
        return img
    elif len(line_arr) == 1:
        return img
    line_arr = np.squeeze(line_arr)
    # Get slope degree to separate 2 group (+ slope , - slope)
    slope_degree = (np.arctan2(line_arr[:, 1] - line_arr[:, 3], line_arr[:, 0] - line_arr[:, 2]) * 180) / np.pi

    stop_arr = line_arr[np.abs(slope_degree) > 173]
    stop_degree = slope_degree[np.abs(slope_degree) > 173]

    S_lines = stop_arr[(stop_degree < 187), :]

    stop_interp = Collect_points(S_lines)

    stop_fit_line = ransac_line_fitting(img, stop_interp)

    S_lane.append(stop_fit_line)
    if len(S_lane) > 10:
        stop_fit_line = smoothing(S_lane, 10)
    stop = draw_stop_line(img, stop_fit_line)

    return stop

def detect_lanes_img(img):
    height, width = img.shape[:2] #360, 480
    # global prediction_buff
    # vertices2 = []
    #img = filter_colors(img)
    # cv2.imshow('1st', img)

    # Set ROI
    vertices = np.array(
        [[(0, 360), (140, 190), (480 - 140, 190), (480 - 0, 360),
          (480 - 100, 360), (300, 340), (180, 340), (100, 360)]],
        dtype = np.int32)
    ROI_img = region_of_interest(img, vertices)
    cv2.imshow('mask', ROI_img)

    # Convert to grayimage
    g_img = grayscale(ROI_img)

    # Apply gaussian filter
    blur_img = gaussian_blur(g_img, 7) #7

    # Apply Canny edge transform
    canny_img = canny(blur_img, 200, 300) #200, 300
    # to except contours of ROI image

    # if prediction_buff == FORWARD & LEFT1 & RIGHT1:
    vertices2 = np.array(
        [[(0+3, 360-2), (140+2, 190+2), (480-140-2, 190+2), (480-0-3, 360-2),
          (480 - 100 + 2, 360 - 2),
          (300 + 2, 340 - 2), (180 - 2, 340-2),
          (100 - 2, 360 - 2)]],
        dtype = np.int32)

    canny_img = region_of_interest(canny_img, vertices2)
    # cv2.imshow('canny', canny_img)

    # Perform hough transform
    # Get first candidates for real lane lines
    line_arr = hough_lines(canny_img, 1, 1 * np.pi / 180, 30, 10, 20)

    # draw_lines(img, line_arr, thickness=2)

    if line_arr is None:
        return img
    elif len(line_arr) == 1:
        return img
    line_arr = np.squeeze(line_arr)
    # Get slope degree to separate 2 group (+ slope , - slope)
    slope_degree = (np.arctan2(line_arr[:, 1] - line_arr[:, 3], line_arr[:, 0] - line_arr[:, 2]) * 180) / np.pi

    # ignore horizontal slope lines
    line_arr = line_arr[np.abs(slope_degree) < 160]
    slope_degree = slope_degree[np.abs(slope_degree) < 160]
    # ignore vertical slope lines
    line_arr = line_arr[np.abs(slope_degree) > 95]
    slope_degree = slope_degree[np.abs(slope_degree) > 95]
    L_lines, R_lines = line_arr[(slope_degree > 0), :], line_arr[(slope_degree < 0), :]
    # print(line_arr.shape,'  ',L_lines.shape,'  ',R_lines.shape)

    # interpolation & collecting points for RANSAC
    L_interp = Collect_points(L_lines)
    R_interp = Collect_points(R_lines)

    # draw_circle(img,L_interp,(255,255,0))
    # draw_circle(img,R_interp,(0,255,255))

    # erase outliers based on best line
    left_fit_line = ransac_line_fitting(img, L_interp)
    right_fit_line = ransac_line_fitting(img, R_interp)

    # smoothing by using previous frames
    L_lane.append(left_fit_line), R_lane.append(right_fit_line)

    if len(L_lane) > 10:
        left_fit_line = smoothing(L_lane, 10)
    if len(R_lane) > 10:
        right_fit_line = smoothing(R_lane, 10)
    final, prediction = draw_fitline(img, left_fit_line, right_fit_line)

    cv2.imshow('result', final)

    return prediction

"""--------------Computer Vision Variable--------------"""
NULL = 0
VARIANCE = 30
SATURATION = 150
# FORWARD_THRESHOLD = 0.3
RED, GREEN, BLUE, YELLOW = (0, 1, 2, 3)
# FORWARD, LEFT, RIGHT = (0, 1, 2)
COLOR = ("RED", "GREEN", "BLUE", "YELLOW")
DIRECTION = ("FORWARD", "LEFT", "RIGHT")
HUE_THRESHOLD = ([4, 176], [40, 80], [110, 130], [20, 40])
"""-----------------------------------------------------"""

global row
global col
global dim

row, col, dim = None, None, None


def rgb_conversion(img):
    return cv2.cvtColor(img.copy(), cv2.COLOR_BGR2RGB)


def hsv_conversion(img):
    return cv2.cvtColor(img.copy(), cv2.COLOR_BGR2HSV)


def gray_conversion(img):
    return cv2.cvtColor(img.copy(), cv2.COLOR_BGR2GRAY)


def color_extract(img, idx):
    result = img.copy()

    for i in range(RED + GREEN + BLUE):
        if i != idx:
            result[:, :, i] = np.zeros([row, col])

    return result


def extract_rgb(img, print_enable=False):
    global row
    global col
    global dim

    row, col, dim = img.shape

    img = rgb_conversion(img)

    # Image Color Separating
    img_red = color_extract(img, RED)
    img_green = color_extract(img, GREEN)
    img_blue = color_extract(img, BLUE)

    if print_enable:
        plt.figure(figsize=(12, 4))
        imgset = [img_red, img_green, img_blue]
        imglabel = ["RED", "GREEN", "BLUE"]

        for idx in range(RED + GREEN + BLUE):
            plt.subplot(1, 3, idx + 1)
            plt.xlabel(imglabel[idx])
            plt.imshow(imgset[idx])
        plt.show()

    return img_red[:, :, RED], img_green[:, :, GREEN], img_blue[:, :, BLUE]


def color_filtering(img, roi=None, print_enable=False):
    global row
    global col
    global dim

    row, col, dim = img.shape

    hsv_img = hsv_conversion(img)
    h, s, v = cv2.split(hsv_img)

    s_cond = (s > SATURATION)
    if roi is RED:
        h_cond = (h < HUE_THRESHOLD[roi][0]) | (h > HUE_THRESHOLD[roi][1])
    else:
        h_cond = (h > HUE_THRESHOLD[roi][0]) & (h < HUE_THRESHOLD[roi][1])

    v[~h_cond], v[~s_cond] = 0, 0
    hsv_image = cv2.merge([h, s, v])
    result = cv2.cvtColor(hsv_image, cv2.COLOR_HSV2BGR)

    # if print_enable:
    #     cv2.imshow('color filtering',result)

    return result


def gaussian_blurring(img, kernel_size=(None, None)):
    return cv2.GaussianBlur(img.copy(), kernel_size, 0)


def canny_edge(img, lth, hth):
    return cv2.Canny(img.copy(), lth, hth)


def histogram_equalization(gray):
    return cv2.equalizeHist(gray)


def hough_transform(img, rho=None, theta=None, threshold=None, mll=None, mlg=None, mode="lineP"):
    if mode == "line":
        return cv2.HoughLines(img.copy(), rho, theta, threshold)
    elif mode == "lineP":
        return cv2.HoughLinesP(img.copy(), rho, theta, threshold, lines=np.array([]),
                               minLineLength=mll, maxLineGap=mlg)
    elif mode == "circle":
        return cv2.HoughCircles(img.copy(), cv2.HOUGH_GRADIENT, dp=1, minDist=80,
                                param1=200, param2=10, minRadius=40, maxRadius=100)


def morphology(self, img, kernel_size=(None, None), mode="opening"):
    kernel = cv2.getStructuringElement(cv2.MORPH_RECT, kernel_size)

    if mode == "opening":
        dst = cv2.erode(img.copy(), kernel)
        return cv2.dilate(dst, kernel)
    elif mode == "closing":
        dst = cv2.dilate(img.copy(), kernel)
        return cv2.erode(dst, kernel)
    elif mode == "gradient":
        return cv2.morphologyEx(img.copy(), cv2.MORPH_GRADIENT, kernel)


def point_analyze(gray, line, point_gap, len_threshold):
    disparity = [0, 0]

    for idx in range(2):
        yplus = line[idx + 1] + point_gap if line[idx + 1] + point_gap < row else row - 1
        yminus = line[idx + 1] - point_gap if line[idx + 1] - point_gap >= 0 else 0

        if yplus < 0 or yminus >= row:
            break
        elif yplus >= row or yminus < 0:
            break

        disparity[idx] = np.abs(gray[yplus][line[idx]] - gray[yminus][line[idx]])

    if np.average(disparity) > len_threshold:
        return True
    else:
        return False


def ROI_cutter(img, point1, point2):
    if img is None:
        print("ROI_cutter: 입력 이미지가 None입니다.")
        return None
    imgBefore = img.copy()
    imgAfter = imgBefore[point1[1]:point2[1], point1[0]:point2[0]]
    imgResize = imgAfter.copy()
    return imgResize

def ROI_cutter2(img, point1, point2):
    if img is None:
        print("ROI_cutter2: 입력 이미지가 None입니다.")
        return None
    imgBefore = img.copy()
    imgAfter = imgBefore[point1[1]:point2[1], point1[0]:point2[0]]
    imgResize = cv2.resize(imgAfter, (640, 480))
    return imgResize


def object_detection(img, sample=0, mode="circle", print_enable=False):
    result = None
    replica = img.copy()

    for color in (RED, YELLOW, GREEN):
        extract = color_filtering(img, roi=color, print_enable=True)
        gray = gray_conversion(extract)
        circles = hough_transform(gray, mode=mode)
        if circles is not None:
            for circle in circles[0]:
                center, count = (int(circle[0]), int(circle[1])), 0

                hsv_img = hsv_conversion(img)
                h, s, v = cv2.split(hsv_img)

                # Searching the surrounding pixels
                for res in range(sample):
                    x, y = int(center[1] - sample / 2), int(center[0] - sample / 2)
                    s_cond = s[x][y] > SATURATION
                    if color is RED:
                        h_cond = (h[x][y] < HUE_THRESHOLD[color][0]) | (h[x][y] > HUE_THRESHOLD[color][1])
                        count += 1 if h_cond and s_cond else count
                    else:
                        h_cond = (h[x][y] > HUE_THRESHOLD[color][0]) & (h[x][y] < HUE_THRESHOLD[color][1])
                        count += 1 if h_cond and s_cond else count

                if count > sample / 2:
                    result = COLOR[color]
                    cv2.circle(replica, center, int(circle[2]), (0, 0, 255), 2)     
    return result


cap = cv2.VideoCapture(1)
cap2 = cv2.VideoCapture(0)


#if not cap.isOpened() or not cap2.isOpened():
    #print("카메라를 열 수 없습니다.")
    #exit()q
###########################################################################################

traffic_light_flag = True
traffic_counter = 0


direc = input("write '1' to watch: , '2' to start: ") # 1 눌러서 카메라 설정 확인 후 파이썬만 다시 돌리고 2 누르면 출발함
if direc == "1" or "2":
    comm.write(direc.encode())


while True:
    ret, frame = cap.read()
    if frame is None:
        print("카메라 1에서 프레임을 읽지 못했습니다.")
        continue

    if frame.shape[0] != 540:  # resizing for challenge video
        frame = cv2.resize(frame, None, fx=3 / 4, fy=3 / 4, interpolation=cv2.INTER_AREA)
    direction = detect_lanes_img(frame)

    if (traffic_counter % 4 == 0):
        ret, frame2 = cap2.read()
        if frame2 is None:
            print("카메라 2에서 프레임을 읽지 못했습니다.")
            continue
        
        imgROI = ROI_cutter(frame2, (0, 360), (300, 480))
        if imgROI is None:
            print("ROI 이미지를 생성할 수 없습니다.")
            continue
        
        imgROI = ROI_cutter2(frame2, (0, 360), (300, 480))
        result = object_detection(img=imgROI, sample=16, print_enable=True)
        cv2.imshow("ROI img", imgROI)

        if result == "RED":
            stop = detect_stop_line(frame)
            if np.any(stop == 1):  # stop 배열의 어느 요소라도 1이면
                direction = 10
                print("정지")
        elif result == "GREEN":
            stop = 0
            direction = 11
            print("출발")
        else:
            stop = 0

    traffic_counter += 1

    if type(direction) != int:
        direction = 3
    if ((direction != d1) & (direction != None) & (direction != 0)):
        print(direction)
        d1 = direction
        if direction == 1:
            direc = "a"
        elif direction == 2:
            direc = "b"
        elif direction == 3:
            direc = "c"
        elif direction == 4:
            direc = "d"
        elif direction == 5:
            direc = "e"
        elif direction == 6:
            direc = "f"
        elif direction == 7:
            direc = "g"
        elif direction == 8:
            direc = "h"
        elif direction == 9:
            direc = "i"
        elif direction == 10:
            direc = "j"
        elif direction == 11:
            direc = "k"

        comm.write(direc.encode())

    if cv2.waitKey(1) & 0xFF == ord('q'):
        direc = '0'
        comm.write(direc.encode())
        break

cap.release()
cap2.release()
cv2.destroyAllWindows()