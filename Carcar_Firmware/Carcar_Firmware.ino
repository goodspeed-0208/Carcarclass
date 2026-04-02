#include <SPI.h>
#include <MFRC522.h>
#include <string.h>
#define CUSTOM_NAME "HM10_12"

const int analognum = 5;
const int analogPin[analognum] = {A7, A6, A5, A4, A3};

const int RST_PIN = 3;
const int SS_PIN = 2;
MFRC522 *mfrc522;

const int PWMA = 10;
const int AIN2 = 6;
const int AIN1 = 7;
const int PWMB = 11;
const int BIN2 = 9;
const int BIN1 = 8;
int MotorR_I3 = BIN1, MotorR_I4 = BIN2, MotorL_I1 = AIN1, MotorL_I2 = AIN2;
int MotorL_PWML = PWMA, MotorR_PWMR = PWMB;
int lastsum = 0;
bool innode = 0;
const int TARGET_FPS = 50;
int targetLoopTime = 1000/TARGET_FPS;
unsigned long lastLoopstartTime = 0;
unsigned long nextLoopTime = 0;
int turntime = 0;
int dir;
bool turning = 0;
bool stop = 0;
// left right forward baackward
int mode[] = {1, 3, 2, 3, 0, 3, 2, 3}; 
int state = 0;

// BLUE TOOTH
long baudRates[] = {9600, 19200, 38400, 57600, 115200, 4800, 2400, 1200, 230400};
bool moduleReady = false;

int tracktime = 0;

void initVar() {
  lastsum = 0;
  innode = 0;
  turntime = 0;
  dir = 3;
  turning = 0;
  stop = 0;
  state = 0;
}

unsigned long sendTime;

void setup() {

  initVar();
  //Serial.begin(9600); // 表示開始傳遞與接收序列埠資料
  initIR();
  initRFID();
  initMotor();
  //Serial.begin(9600);
  initBlueTooth();
  stop = 1;
}

bool test = 0;
void loop(){
  if (millis() > 10000 && !test) {
    test = 1;
    sendTime = millis();
    Serial3.println("finish init");
    Serial.println("send msg");
    Serial.println(sendTime);
  }

  unsigned long currentTime = millis();

  if (currentTime >= nextLoopTime) {
    int deltatime = currentTime - lastLoopstartTime;
    lastLoopstartTime = currentTime;
    
    // 設定下一次執行目標時間
    nextLoopTime = currentTime + targetLoopTime;

    Tracking(deltatime);
    readRFID();
    //readIR();
  }

  if (Serial3.available()) {
    String command = Serial3.readString();
    Serial.println(command);
    if (command == "receive init") {
      Serial.print(millis() - sendTime);
      Serial3.print(millis() - sendTime);
    }
    if(command == "e") stop = 1; // end
    else if (command == "s") { // start
      initVar();
    }
  }
}
