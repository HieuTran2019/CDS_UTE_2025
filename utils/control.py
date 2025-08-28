import serial
import time

# Mở cổng Serial khi module được load
ser = serial.Serial('COM4', 115200, timeout=1)
time.sleep(2)  # Chờ mạch khởi động

def control(speed=0, angle=0):
    """
    Gửi lệnh điều khiển ESC (speed) và Servo (angle) qua Serial.
    speed: -25..25
    angle: -25..25
    """
    speed = max(min(speed, 25), -25)
    angle = max(min(angle, 25), -25)

    cmd = f"{speed},{angle}\n"
    try:
        ser.write(cmd.encode())
        print(f"Sent -> Speed: {speed}, Angle: {angle}")
    except serial.SerialException as e:
        print(f"Error sending data: {e}")

def close_serial():
    """Đóng cổng serial"""
    ser.close()

# Không chạy loop khi import
if __name__ == "__main__":
    try:
        while True:
            try:
                speed, angle = map(int, input("Enter speed (-25..25) and angle (-25..25): ").split())
            except ValueError:
                print("Invalid input. Please enter two integers.")
                continue
            control(speed, angle)
            time.sleep(0.05)
    except KeyboardInterrupt:
        print("\nProgram terminated by user.")
    finally:
        close_serial()
