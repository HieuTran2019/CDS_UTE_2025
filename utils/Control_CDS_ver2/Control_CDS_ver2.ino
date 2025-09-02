#include <Servo.h>

// ======================= CẤU HÌNH =======================

// ESC
Servo esc;
const int escPin = 5;
bool wasForward = true;

// Servo lái
Servo steering;
const int servoPin = 4;

// Ngắt ngoài
volatile bool isRunning = false;
unsigned long lastInterruptStart = 0;
unsigned long lastInterruptStop = 0;

// Giới hạn xung PWM ESC
const int PWM_NEUTRAL     = 1500;   //  Giá trị neural
const int PWM_FORWARD_MIN = 1550;   // Ngưỡng bắt đầu tiến
const int PWM_FORWARD_MAX = 1700;   // Giới hạn tốc độ tối đa - Để an toàn không lên 2000 (2000 = max speed)
const int PWM_REVERSE_MIN = 1250;   // Giới hạn lùi 
const int PWM_REVERSE_MAX = 1350;   // Ngưỡng bắt đầu lùi

// Giới hạn servo lái
//  Giới hạn góc lái từ từ [30 : 150], ko để dưới 30 hoặc trên 150 sẽ làm gãy tay arm servo và hệ thống lái 
const int SERVO_LEFT_MAX  = 50;     // Góc trái
const int SERVO_CENTER    = 90;     // Trung tâm
const int SERVO_RIGHT_MAX = 130;    // Góc phải

// ======================= HÀM XỬ LÝ NGẮT =======================

// Hàm xử lý ngắt ngoài - Start
void startProgram() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptStart > 200) { // debounce
    isRunning = true;
    lastInterruptStart = currentTime;
  }
}

// Hàm xử lý ngắt ngoài - Stop
void stopProgram() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptStop > 200) { // debounce
    isRunning = false;
    lastInterruptStop = currentTime;
  }
}

// ======================= HÀM ĐIỀU KHIỂN =======================

// Điều khiển ESC
void controlESC(int speed) {
  speed = constrain(speed, -25, 25);

  if (speed == 0) {
    esc.writeMicroseconds(PWM_NEUTRAL);
    return;
  }

  if (speed > 0) {
    int pulse = map(speed, 1, 25, PWM_FORWARD_MIN, PWM_FORWARD_MAX);
    esc.writeMicroseconds(pulse);
    wasForward = true;
  } else {
    // Reverse ESC cần tín hiệu mồi lùi khoảng 1200ms đổ xuống rồi sau đó trả về khoảng 1500ms rồi mới bắt đầu lùi được
    if (wasForward) {
      esc.writeMicroseconds(1200); // chuyển ESC sang chế độ reverse
      delay(300);
      esc.writeMicroseconds(PWM_NEUTRAL);
      delay(300);
      wasForward = false;
    }
    int pulse = map(speed, -25, -1, PWM_REVERSE_MIN, PWM_REVERSE_MAX);
    esc.writeMicroseconds(pulse);
  }
}

// Điều khiển servo lái
void controlServo(int angle) {
  angle = constrain(angle, -25, 25);
  int position = map(angle, -25, 25, SERVO_LEFT_MAX, SERVO_RIGHT_MAX);
  steering.write(position);
}

// ======================= SETUP =======================

void setup() {
  Serial.begin(115200);

  // Nút nhấn nối về GND, bật pull-up nội
  pinMode(D7, INPUT_PULLUP); // Start
  pinMode(D6, INPUT_PULLUP); // Stop

  // LED trạng thái
  pinMode(LED_BUILTIN, OUTPUT);

  // Gắn ngắt ngoài cho nút
  attachInterrupt(digitalPinToInterrupt(D7), startProgram, FALLING);
  attachInterrupt(digitalPinToInterrupt(D6), stopProgram, FALLING);

  // Khởi tạo ESC và servo
  esc.attach(escPin, 1000, 2000);
  esc.writeMicroseconds(PWM_NEUTRAL);  // Neutral ESC lúc đầu
  delay(3000);

  steering.attach(servoPin);

  // Tắt LED lúc đầu
  digitalWrite(LED_BUILTIN, LOW);
}

// ======================= LOOP =======================

void loop() {
  // LED báo trạng thái chạy/dừng
  digitalWrite(LED_BUILTIN, isRunning ? HIGH : LOW);

  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    if (input.length() > 0) {
      int commaIndex = input.indexOf(',');
      if (commaIndex > 0) {
        int speed = input.substring(0, commaIndex).toInt();
        int angle = input.substring(commaIndex + 1).toInt();

        if (isRunning) {
          controlESC(speed);
          controlServo(angle);
        } else {
          controlESC(0);
          controlServo(0);
        }
      }
    }
  } else {
    // Nếu không nhận lệnh nào, và dừng, đảm bảo ESC & servo ở 0
    if (!isRunning) {
      controlESC(0);
      controlServo(0);
    }
  }
}
