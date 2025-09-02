#include <Servo.h>

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

// Điều khiển ESC
void controlESC(int speed) {
  int pulse = 1500; // neutral

  if (speed > 25)  speed = 25;
  if (speed < -25) speed = -25;

  if (speed > 0) {
    pulse = 1550 + speed * 4; // Forward
    esc.writeMicroseconds(pulse);
    wasForward = true;
  }
  else if (speed == 0) {
    esc.writeMicroseconds(1500); // Neutral
  }
  else {
    // Reverse
    if (wasForward) {
      esc.writeMicroseconds(1200);
      delay(300);
      esc.writeMicroseconds(1500);
      delay(300);
      wasForward = false;
    }
    pulse = 1350 + speed * 4;
    esc.writeMicroseconds(pulse);
  }
}

// Điều khiển servo lái
void controlServo(int angle) {
  angle = constrain(angle, -25, 25);
  // int position = map(angle, -25, 25, 60, 120); // tăng range của servo lên (50 90 130 )   (40 90 140)
  int position = map(angle, -25, 25, 50, 130); // tăng range của servo lên (50 90 130 )   (40 90 140)
  // int position = map(angle, -25, 25, 40, 140); // tăng range của servo lên (50 90 130 )   (40 90 140)

  steering.write(position);
}

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
  esc.writeMicroseconds(1500);  // Neutral ESC lúc đầu
  delay(3000);

  steering.attach(servoPin);

  // Tắt LED lúc đầu
  digitalWrite(LED_BUILTIN, LOW);
}

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
