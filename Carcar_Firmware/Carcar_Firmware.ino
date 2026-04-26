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

String btCommandBuffer = ""; //Bluetooth 用

const int direction_num = 8;

enum Direction {
  FORWARD = 0,
  LEFT = 1,
  RIGHT = 2,
  BACKWARD = 3,
  TURN_BACK = 4,
  LEFT_AFTER_BACKWARD = 5,
  RIGHT_AFTER_BACKWARD = 6,
  STAY_STOP = 7,
  WAIT_FOR_COMMAND = 8,
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
    case WAIT_FOR_COMMAND: return "no command";
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

  void stop(int deltaTime = targetLoopTime);  //目前放在Navigation.ino
  void restart();

  void reportData();
  void printMotionData(String actionName, MotionData data);

  void reading();
  void readIR();
  void readRFID();
  void navigating(int deltaTime);
  void adjust_motor_error(int deltaTime);

  /*long sum_vL = 0;
  long sum_vR = 0;
  double averagevL = 0;
  double averagevR = 0;
  long trackCount = 0;*/

  Direction next_dir;

  //RFID
  

  //tracking
  

  // adjust
  //bool adjust_start = 0;

  // --- Turn Test Variables & Functions ---
  /*enum TestState { TEST_WAIT,
                   TEST_TRACK,
                   TEST_TURN };
  TestState testState = TEST_WAIT;

  char testDir = 'L';
  int testOuterSpeed = 0;
  int testInnerSpeed = 0;
  unsigned long turnStartTime = 0;

  void start_turn_test(char dir, int outer, float ratio);
  void run_turn_test(int deltaTime);*/

private:
  void initIR();
  void initRFID();
  void initMotor();

  void goForward();
  void turnleft();
  void turnright();
  void turnback();
  void goBackward();
  void turnleft_after_backward();
  void turnright_after_backward();
  void wait_for_command();
  void Tracking(int deltaTime);
  void MotorWriting(int deltaTime);

private:
  int MotorR_I3 = BIN1, MotorR_I4 = BIN2, MotorL_I1 = AIN1, MotorL_I2 = AIN2;
  int MotorL_PWML = PWMA, MotorR_PWMR = PWMB;

  //RFID
  int RFID_SS_PIN = SS_PIN;
  int RFID_RST_PIN = RST_PIN;
  MFRC522 *mfrc522;
  bool isFindingRFID = false;
  unsigned long RFID_startTime = 0;
  unsigned long Max_RFID_findtime = 400;
  String visitedUIDs[30]; 
  int visitedCount = 0;
  const int MAX_VISITED = 30;
  //IR
  int IRvalue[analognum] = { 0, 0, 0, 0, 0 };
  bool IRisBlack[analognum] = { 0, 0, 0, 0, 0 };
  int IRisBlackValue[analognum] = { 35, 40, 35, 40, 40 };
  int IRsum = 0;
  //int IRtracktime = 0;

  //Motor
  int target_motor_vL = 0;
  int target_motor_vR = 0;
  int last_motor_vL = 0;
  int last_motor_vR = 0;
  //int motor_error = 3;
  int back_motor_error = 2;
  double maxAcceleration = 512.0 / targetLoopTime;  //11基本上就是沒有最大加速度限制


  //Navigation(關注在前進左右倒退迴轉的模式)(在Navigation.ino)
  int forwardspeed = 160;
  int backwardspeed_first = forwardspeed;
  int backwardspeed_second = forwardspeed / 1.6;
  int turnBackSpeed_first = 160;
  int turnBackSpeed_second = 80;
  int turnOuterSpeed = forwardspeed;
  int turnInnerSpeed = int((turnOuterSpeed - 14) * 0.15) + 14;
  int turnOuterSpeed_back = forwardspeed;
  int turnInnerSpeed_back = 0;
  int inUturnSpeed_decel = 120;

  bool isRunning = 0;
  bool isInnode = 0;
  bool turning = 0;

  unsigned long turntime = 0;
  unsigned long Min_forward_turntime = 15000 / forwardspeed;
  unsigned long Min_rightleft_turntime = 60000 / turnOuterSpeed;
  unsigned long Min_turnback_turntime = 50000 / turnBackSpeed_first;
  unsigned long Min_backward_turntime = 112000 / forwardspeed;
  unsigned long inUturnDecelTime = 57000 / forwardspeed;

  bool extremeModeOn = true;     // 是否開啟極限模式
  int extremeSpeed = 200;        // 極限直線速度
  unsigned long extremeAccelDelay = 250; // 出彎後延遲 0.4s (400ms) 加速
  unsigned long extremeDecelDelay = 300; // 即將入彎前，離開上個節點 0.25s (250ms) 後減速

  bool lastActionWasTurn = true; // 紀錄上一個動作是否為轉彎 (開局預設為 true 比較安全)
  bool lastActionWasUTurn = false;
  int currentSegmentType = 0; //這次的直走類型是甚麼

  Direction dir;  // left right forward baackward
  /*Direction mode[8] = { RIGHT, TURN_BACK, FORWARD, TURN_BACK, LEFT, TURN_BACK, FORWARD, TURN_BACK };
  int modeState = 0;*/
  
  //data
  unsigned long start_time = 0;
  unsigned long motion_startTime = 0;
  MotionData trackingData[6];
  MotionData turningData[direction_num];

  //Tracking(關注在前進(或後退)的循跡演算法)(在Navigation.ino)
  // PID for tracking
  double Kp = 15;
  double Ki = 0;
  double Kd = 1500;

  double exKp = 10;
  double exKi = 0;
  double exKd = 1800;

  double lastError = 0;
  double integral = 0;


  //adjust
  //unsigned long adjust_start_time;

  //class CarCar裡不會有bluetooth，因為bluetooth是車車與電腦的溝通橋樑
};

CarCar mycar;

//unsigned long sendTime;
/*unsigned long IRsendtime = 500;
unsigned long IRcurrenttime = 0;
unsigned long IRnexttime = 0;*/

void setup() {
  //Serial.begin(9600); // 表示開始傳遞與接收序列埠資料
  //Serial.begin(9600);
  initBlueTooth();
  mycar.begin();
  Serial3.setTimeout(80);  // Minimize timeout to prevent blocking if '\n' is missing
  btCommandBuffer.reserve(50);

  lastLoopstartTime = millis(); // 防止setup過久，迴圈一直執行
  nextLoopTime = lastLoopstartTime + targetLoopTime;
}

/*bool test = 0;
unsigned long maxLoopDuration = 0;

unsigned long nextSendTime = 0;
unsigned long deltaSendTime = 1000;*/

void loop() {
  /*if (millis() > 10000 && !test) {
    test = 1;
    sendTime = millis();
    Serial3.println("finish init");
    Serial.println("send msg");
    Serial.println(sendTime);
  }*/

  while (Serial3.available() > 0) {
    char c = Serial3.read();
    if (c == '\n') {
      btCommandBuffer.trim(); 
      processBluetoothCommand(btCommandBuffer);
      btCommandBuffer = ""; // 清空準備接下一道指令
    } else {
      btCommandBuffer += c;
    }
  }

  unsigned long currentTime = millis();

  mycar.readRFID();

  if (currentTime >= nextLoopTime) {
    unsigned long deltatime = currentTime - lastLoopstartTime;
    lastLoopstartTime = currentTime;

    // 設定下一次執行目標時間
    nextLoopTime += targetLoopTime;

    mycar.readIR();
    mycar.navigating(deltatime);
    //mycar.adjust_motor_error(deltatime);
    //mycar.run_turn_test(deltatime);

    //檢查loop花費時間
    //maxLoopDuration = max(maxLoopDuration, millis() - currentTime);
  }

  /*if(currentTime >= nextSendTime){
      nextSendTime += deltaSendTime;

      Serial.print("max loop duration: ");
      Serial.println(maxLoopDuration);

      if(maxLoopDuration > 50){
        Serial3.print("max loop: ");
        Serial3.println(maxLoopDuration);
      }
      
      maxLoopDuration = 0;
  }*/

  /*if (Serial3.available()) {
    String cmd = Serial3.readStringUntil('\n');
    processBluetoothCommand(cmd);
  }*/
}
