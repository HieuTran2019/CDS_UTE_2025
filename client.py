# import library
from utils.control import control, close_serial
import cv2
import time
import numpy as np

# ===== Khởi tạo biến PID =====
pre_t = time.time()
error_arr = np.zeros(10)


# code function
def PID(error, p, i, d):
    global pre_t, error_arr

    # Cập nhật error history
    error_arr[1:] = error_arr[0:-1]
    error_arr[0] = error

    # P, I, D
    P = error * p
    delta_t = time.time() - pre_t
    pre_t = time.time()

    D = (error - error_arr[1]) / delta_t * d if delta_t > 0 else 0
    I = np.sum(error_arr) * delta_t * i

    angle = P + I + D

    # Giới hạn [-25, 25]
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

            # ===== Workspace: chạy chương trình ở đây =====
            # Ví dụ: luôn chạy thẳng
            speed = 20
            error = 0  # giả định không lệch làn
            angle = PID(error, p=0.5, i=0.01, d=0.1)

            control(speed, angle)

            # cv2.imshow("Camera", image)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
            # =====================================

            time.sleep(0.05)

    except KeyboardInterrupt:
        print("\n🛑 Client stopped.")
    finally:
        cam.release()
        cv2.destroyAllWindows()
        close_serial()
