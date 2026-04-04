#include <SPI.h>
#include <MFRC522.h>
#include <string.h>
#define CUSTOM_NAME "HM10_12"

const int analognum = 5;
const int analogPin[analognum] = { A7, A6, A5, A4, A3 };

const int RST_PIN = 3;
const int SS_PIN = 2;
const int PWMA = 10;
const int AIN2 = 6;
const int AIN1 = 7;
const int PWMB = 11;
const int BIN2 = 9;
const int BIN1 = 8;

long baudRates[] = { 9600, 19200, 38400, 57600, 115200, 4800, 2400, 1200, 230400 };
bool blueToothModuleReady = false;

const int TARGET_FPS = 50;
int targetLoopTime = 1000 / TARGET_FPS;
unsigned long lastLoopstartTime = 0;
unsigned long nextLoopTime = 0;

class CarCar {
public:
  CarCar();
  ~CarCar();

  void begin();

  void stop();  //目前放在Navigation.ino
  void restart();

  void reading();
  void navigating(int deltaTime);
  void adjust_motor_error();

  enum Direction {
    FORWARD = 0,
    LEFT = 1,
    RIGHT = 2,
    BACKWARD = 3,
    TURN_BACK = 4,
    STAY_STOP = -1
  };

  int forwardspeed = 100;
  int backwardspeed = 100;
  int turnBackSpeed = 50;
  int turnOuterSpeed = 100;
  int turnInnerSpeed = 25;

  int motor_error = 2;

private:
  void initIR();
  void initRFID();
  void initMotor();

  void readIR();
  void readRFID();

  void goForward();
  void turnleft();
  void turnright();
  void turnback();
  void goBackward();
  void Tracking(int deltaTime);
  void MotorWriting();

private:
  int MotorR_I3 = BIN1, MotorR_I4 = BIN2, MotorL_I1 = AIN1, MotorL_I2 = AIN2;
  int MotorL_PWML = PWMA, MotorR_PWMR = PWMB;

  //RFID
  int RFID_SS_PIN = SS_PIN;
  int RFID_RST_PIN = RST_PIN;
  MFRC522 *mfrc522;

  //IR
  int IRvalue[analognum] = { 0, 0, 0, 0, 0 };
  bool IRisBlack[analognum] = { 0, 0, 0, 0, 0 };
  int IRsum = 0;
  int IRtracktime = 0;

  //Motor
  int motor_vL = 0;
  int motor_vR = 0;


  //Navigation(關注在前進左右倒退迴轉的模式)(在Navigation.ino)
  bool isRunning = 0;
  bool isInnode = 0;
  bool turning = 0;
  int turntime = 0;
  int Min_rightleft_turntime = 300;
  int Min_turnback_turntime = 500;
  int Min_backward_turntime = 800;
  Direction dir;  // left right forward baackward
  Direction mode[8] = { RIGHT, TURN_BACK, FORWARD, TURN_BACK, LEFT, TURN_BACK, FORWARD, TURN_BACK };
  int modeState = 0;

  //Tracking(關注在前進(或後退)的循跡演算法)(在Navigation.ino)


  //class CarCar裡不會有bluetooth，因為bluetooth是車車與電腦的溝通橋樑
};

CarCar mycar;

unsigned long sendTime;

void setup() {
  //Serial.begin(9600); // 表示開始傳遞與接收序列埠資料
  //Serial.begin(9600);
  initBlueTooth();
  mycar.begin();
  Serial3.setTimeout(10);  // Minimize timeout to prevent blocking if '\n' is missing
}

bool test = 0;
void loop() {
  if (millis() > 10000 && !test) {
    test = 1;
    sendTime = millis();
    Serial3.println("finish init");
    Serial.println("send msg");
    Serial.println(sendTime);
  }

  unsigned long currentTime = millis();

  if (currentTime >= nextLoopTime) {
    unsigned long deltatime = currentTime - lastLoopstartTime;
    lastLoopstartTime = currentTime;

    // 設定下一次執行目標時間
    nextLoopTime = currentTime + targetLoopTime;

    mycar.reading();
    //mycar.navigating(deltatime);
    mycar.adjust_motor_error();
  }

  if (Serial3.available()) {
    String command = Serial3.readStringUntil('\n');  //此處原本為String command = Serial3.readString();
    command.trim();
    Serial.println(command);
    if (command == "receive init") {
      Serial.print(millis() - sendTime);
      Serial3.print(millis() - sendTime);
    }
    if (command == "e") mycar.stop();  // end
    else if (command == "s") {         // start
      mycar.restart();
    } else if (command.indexOf(' ') != -1) {
      int spaceIndex = command.indexOf(' ');

      // Extract and convert to integers
      mycar.forwardspeed = command.substring(0, spaceIndex).toInt();
      mycar.motor_error = command.substring(spaceIndex + 1).toInt();

      // Call restart to set isRunning = 1 and reset parameters
      mycar.restart();

      // Print to USB Serial monitor for debugging
      Serial.print("Calibration started. Speed: ");
      Serial.print(mycar.forwardspeed);
      Serial.print(" | Error: ");
      Serial.println(mycar.motor_error);

      // Send to Bluetooth terminal via Serial3
      Serial3.print("Calibration started. Speed: ");
      Serial3.print(mycar.forwardspeed);
      Serial3.print(" | Error: ");
      Serial3.println(mycar.motor_error);
    }
  }
}
