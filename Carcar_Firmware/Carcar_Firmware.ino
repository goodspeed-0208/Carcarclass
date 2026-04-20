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

const int TARGET_FPS = 20;
int targetLoopTime = 1000 / TARGET_FPS;
unsigned long lastLoopstartTime = 0;
unsigned long nextLoopTime = 0;

const unsigned long total_time = 65000;

const int direction_num = 8;

enum Direction {
  FORWARD = 0,
  LEFT = 1,
  RIGHT = 2,
  BACKWARD = 3,
  TURN_BACK = 4,
  LEFT_AFTER_BACKWARD = 5,
  RIGHT_AFTER_BACKWARD = 6,
  STAY_STOP = 7
};

String getDirString(Direction d) {
  switch (d) {
    case FORWARD: return "F";
    case LEFT: return "L";
    case RIGHT: return "R";
    case BACKWARD: return "B";
    case TURN_BACK: return "T";
    case LEFT_AFTER_BACKWARD: return "LB";
    case RIGHT_AFTER_BACKWARD: return "RB";
    case STAY_STOP: return "STOP";
    default: return "???";
  }
}

struct MotionData {
  unsigned long totalTime;
  int count;
  unsigned long maxTime;
  unsigned long minTime;

  MotionData() {
    reset();
  }

  void reset() {
    totalTime = 0;
    count = 0;
    maxTime = 0;
    minTime = 4294967295;
  }

  void update(unsigned long motion_duration) {
    if (motion_duration <= 100) return;
    count++;
    totalTime += motion_duration;
    maxTime = max(maxTime, motion_duration);
    minTime = min(minTime, motion_duration);
  }
};

class CarCar {
public:
  CarCar();
  ~CarCar();

  void begin();

  void stop();  //目前放在Navigation.ino
  void restart();

  void reportData();
  void printMotionData(String actionName, MotionData data);

  void reading();
  void navigating(int deltaTime);
  void adjust_motor_error(int deltaTime);


  int forwardspeed = 150;
  int backwardspeed = forwardspeed;
  int turnBackSpeed = forwardspeed / 2;
  int turnOuterSpeed = forwardspeed;
  int turnInnerSpeed = int((turnOuterSpeed - 14) * 0.15) + 14;
  int turnOuterSpeed_back = forwardspeed;
  int turnInnerSpeed_back = 0;
  long sum_vL = 0;
  long sum_vR = 0;
  double averagevL = 0;
  double averagevR = 0;
  long trackCount = 0;

  Direction next_dir;

  double maxAcceleration = 512 / targetLoopTime;  //11基本上就是沒有最大加速度限制

  int motor_error = 3;

  //tracking
  double Kp = 15;
  double Ki = 0;
  double Kd = 1500;

  // adjust
  bool adjust_start = 0;

  // --- Turn Test Variables & Functions ---
  enum TestState { TEST_WAIT,
                   TEST_TRACK,
                   TEST_TURN };
  TestState testState = TEST_WAIT;

  char testDir = 'L';
  int testOuterSpeed = 0;
  int testInnerSpeed = 0;
  unsigned long turnStartTime = 0;

  void start_turn_test(char dir, int outer, float ratio);
  void run_turn_test(int deltaTime);

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
  void turnleft_after_backward();
  void turnright_after_backward();
  void Tracking(int deltaTime);
  void MotorWriting(int deltaTime);

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
  int IRisBlackValue[analognum] = { 35, 40, 35, 40, 40 };
  int IRsum = 0;
  int IRtracktime = 0;

  //Motor
  int target_motor_vL = 0;
  int target_motor_vR = 0;
  int last_motor_vL = 0;
  int last_motor_vR = 0;


  //Navigation(關注在前進左右倒退迴轉的模式)(在Navigation.ino)
  bool isRunning = 0;
  bool isInnode = 0;
  bool turning = 0;
  int turntime = 0;
  int Min_rightleft_turntime = 40000 / forwardspeed;
  int Min_turnback_turntime = 80000 / forwardspeed;
  int Min_backward_turntime = 800;
  Direction dir;  // left right forward baackward
  Direction mode[8] = { RIGHT, TURN_BACK, FORWARD, TURN_BACK, LEFT, TURN_BACK, FORWARD, TURN_BACK };
  int modeState = 0;

  //data
  unsigned long start_time = 0;
  unsigned long motion_startTime = 0;
  MotionData trackingData;
  MotionData turningData[direction_num];


  //Tracking(關注在前進(或後退)的循跡演算法)(在Navigation.ino)
  // PID for tracking
  double lastError = 0;
  double integral = 0;


  //adjust
  unsigned long adjust_start_time;



  //class CarCar裡不會有bluetooth，因為bluetooth是車車與電腦的溝通橋樑
};

CarCar mycar;

unsigned long sendTime;
unsigned long IRsendtime = 500;
unsigned long IRcurrenttime = 0;
unsigned long IRnexttime = 0;

void setup() {
  //Serial.begin(9600); // 表示開始傳遞與接收序列埠資料
  //Serial.begin(9600);
  initBlueTooth();
  mycar.begin();
  Serial3.setTimeout(80);  // Minimize timeout to prevent blocking if '\n' is missing
}

bool test = 0;
unsigned long maxLoopDuration = 0;

unsigned long nextSendTime = 0;
unsigned long deltaSendTime = 1000;

void loop() {
  /*if (millis() > 10000 && !test) {
    test = 1;
    sendTime = millis();
    Serial3.println("finish init");
    Serial.println("send msg");
    Serial.println(sendTime);
  }*/

  unsigned long currentTime = millis();

  if (currentTime >= nextLoopTime) {
    unsigned long deltatime = currentTime - lastLoopstartTime;
    lastLoopstartTime = currentTime;

    // 設定下一次執行目標時間
    nextLoopTime += targetLoopTime;

    mycar.reading();
    mycar.navigating(deltatime);
    //mycar.adjust_motor_error(deltatime);
    //mycar.run_turn_test(deltatime);

    //檢查loop花費時間
    maxLoopDuration = max(maxLoopDuration, millis() - currentTime);
  }

  /*if(currentTime >= nextSendTime){
      nextSendTime += deltaSendTime;

      Serial.print("max loop duration: ");
      Serial.println(maxLoopDuration);

      Serial3.print("max loop duration: ");
      Serial3.println(maxLoopDuration);
      maxLoopDuration = 0;
  }*/

  if (Serial3.available()) {
    String cmd = Serial3.readStringUntil('\n');
    processBluetoothCommand(cmd);
  }
}
