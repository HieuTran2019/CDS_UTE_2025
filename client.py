# import library
from utils.control import control, close_serial
import cv2
import time
import numpy as np


# define variables
speed = 0
angle = 0

pre_t = time.time()
error_arr = np.zeros(10)   # lưu 10 giá trị error gần nhất


# code function
def PID(error, p, i, d):
    global pre_t, error_arr
    error_arr[1:] = error_arr[0:-1]
    error_arr[0] = error
    P = error * p
    delta_t = time.time() - pre_t
    pre_t = time.time()
    if delta_t != 0:
        D = (error - error_arr[1]) / delta_t * d
    else:
        D = 0
    I = np.sum(error_arr) * delta_t * i
    angle = P + I + D
    if abs(angle) > 25:
        angle = np.sign(angle) * 25
    return int(angle)




# ================ main ===============
if __name__ == "__main__":
    cam = cv2.VideoCapture(0)
    if not cam.isOpened():
        print("❌ Không mở được camera.")
        exit()

    try:
        while True:
            ret, image = cam.read()
            if not ret:
                print("❌ Không nhận được frame từ camera.")
                break

            # ================== HƯỚNG DẪN ==================
            # - Chương trình đưa cho bạn 1 giá trị đầu vào:
            #   * image: hình ảnh trả về từ xe
            #
            #   * Hàm điều khiển control(speed, angle)
            #     Trong đó:
            #       + angle (góc điều khiển): [-25, 25]
            #           NOTE: âm là góc trái, dương là góc phải
            #       + speed (tốc độ điều khiển): [-25, 25]
            #           NOTE: âm là lùi, dương là tiến
            # =================================================

            # ===== Workspace: chạy chương trình ở đây =====
            speed = 20
            error = 0  # tạm thời giả định xe không lệch
            angle = PID(error, p=0.5, i=0.01, d=0.1)  # PID điều chỉnh góc

            control(speed, angle)
            # cv2.imshow("Camera", image)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
            # ==============================================

            time.sleep(0.05)

    except KeyboardInterrupt:
        print("\n🛑 Client stopped.")
    finally:
        cam.release()
        cv2.destroyAllWindows()
        close_serial()
