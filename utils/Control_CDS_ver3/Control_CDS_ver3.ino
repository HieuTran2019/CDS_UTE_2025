#include <Servo.h>

// ======================= CẤU HÌNH =======================

// ESC
Servo esc;
const int escPin = 5;
bool wasForward = true;

// Servo lái
Servo steering;
const int servoPin = 4;

// Nút bấm
const int buttonStartPin = D7;
const int buttonStopPin  = D6;

// Trạng thái chạy/dừng
bool isRunning = false;

// Giới hạn xung PWM ESC
const int PWM_NEUTRAL     = 1500;   // Giá trị neutral
const int PWM_FORWARD_MIN = 1550;   // Ngưỡng bắt đầu tiến
const int PWM_FORWARD_MAX = 1700;   // Giới hạn tốc độ tối đa
const int PWM_REVERSE_MIN = 1250;   // Giới hạn lùi 
const int PWM_REVERSE_MAX = 1350;   // Ngưỡng bắt đầu lùi

// Giới hạn servo lái
const int SERVO_LEFT_MAX  = 50;     // Góc trái
const int SERVO_CENTER    = 90;     // Trung tâm
const int SERVO_RIGHT_MAX = 130;    // Góc phải

// ======================= HÀM CHECK NÚT (DEBOUNCE) =======================

void checkButtons() {
  static bool lastStartState = HIGH;
  static bool lastStopState  = HIGH;
  static unsigned long lastDebounce = 0;
  const int debounceDelay = 50;

  bool readingStart = digitalRead(buttonStartPin);
  bool readingStop  = digitalRead(buttonStopPin);

  // Xử lý nút Start
  if (readingStart != lastStartState) {
    lastDebounce = millis();
  }
  if ((millis() - lastDebounce) > debounceDelay) {
    if (readingStart == LOW) {
      isRunning = true;
    }
  }
  lastStartState = readingStart;

  // Xử lý nút Stop
  if (readingStop != lastStopState) {
    lastDebounce = millis();
  }
  if ((millis() - lastDebounce) > debounceDelay) {
    if (readingStop == LOW) {
      isRunning = false;
    }
  }
  lastStopState = readingStop;
}

// ======================= HÀM ĐIỀU KHIỂN =======================

// Điều khiển ESC
void controlESC(int speed) {
  speed = constrain(speed, -25, 50);

  if (speed == 0) {
    esc.writeMicroseconds(PWM_NEUTRAL);
    return;
  }

  if (speed > 0) {
    int pulse = map(speed, 1, 50, PWM_FORWARD_MIN, PWM_FORWARD_MAX);
    esc.writeMicroseconds(pulse);
    wasForward = true;
  } else {
    // Reverse ESC cần tín hiệu mồi
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
  pinMode(buttonStartPin, INPUT_PULLUP);
  pinMode(buttonStopPin, INPUT_PULLUP);

  // LED trạng thái
  pinMode(LED_BUILTIN, OUTPUT);

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
  // Kiểm tra nút nhấn
  checkButtons();

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
