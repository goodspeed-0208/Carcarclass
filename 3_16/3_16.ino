#include <SPI.h>
#include <MFRC522.h>

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
int mode[] = {1, 3, 2, 3, 0, -1}; 
int state = 0;

void initIR() {
  for (int i = 0 ; i < analognum ; i++) pinMode(analogPin[i] , INPUT); // 目前預設該接腳作為輸入
}

int* testIR() {
  int r[5];
  for (int i = 0 ; i < analognum ; i++) {
    int sensorValue = analogRead(analogPin[i]); // 宣告 sensorValue 這變數是整數(Integer)
    r[i] = sensorValue;
    Serial.println(sensorValue); // 將數值印出來
  }
  Serial.println("__________________");
  //delay (2000); // 延遲 2 秒
  return r;
}

void initRFID() {

  SPI.begin();
  mfrc522 = new MFRC522(SS_PIN, RST_PIN);
  // 請系統去要一塊記憶體空間，後面呼叫它的建構函式
  // 將(SS, RST) 當成參數傳進去初始化。
  mfrc522->PCD_Init();
  /* 初始化MFRC522讀卡機 PCD_Init 模組。 -> 表示：
  透過記憶體位置，找到 mfrc522 這物件，再翻其內容。*/
  Serial.println(F("Read UID on a MIFARE PICC:"));
}

void testRFID() {
  if(!mfrc522->PICC_IsNewCardPresent()) {
    goto FuncEnd;
  } //PICC_IsNewCardPresent()：是否感應到新的卡片?
  if(!mfrc522->PICC_ReadCardSerial()) {
    goto FuncEnd;
  } //PICC_ReadCardSerial()：是否成功讀取資料?
  Serial.println(F("**Card Detected:**"));
  mfrc522->PICC_DumpDetailsToSerial(&(mfrc522->uid)); //讀出 UID
  mfrc522->PICC_HaltA(); // 讓同一張卡片進入停止模式 (只顯示一次)
  mfrc522->PCD_StopCrypto1(); // 停止 Crypto1
  FuncEnd:; // goto 跳到這.
}

void initMotor() {
  pinMode(PWMA, OUTPUT);
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(PWMB, OUTPUT);
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
}

/*oid testMotor() {
  analogWrite(PWMA, 255);    // run motor
  digitalWrite(AIN1, HIGH);
  digitalWrite(AIN2, LOW);
  analogWrite(PWMB, 255);    // run motor
  digitalWrite(BIN1, HIGH);
  digitalWrite(BIN2, LOW);
}*/

void MotorWriting(double vL, double vR) {
  if (vL > 0) vL += 10;
  else if (vL < 0) vL -= 10;
  if (vL > 255) vL = 255;
  if (vR >= 0) {
    digitalWrite(MotorR_I3, LOW);
    digitalWrite(MotorR_I4, HIGH);
    //這邊的Motor第幾個對應到的High/Low是助教的車對應到的，請自己測試自己車該怎麼填！
  } else if (vR < 0) {
    digitalWrite(MotorR_I3, HIGH);
    digitalWrite(MotorR_I4, LOW);
    vR = -vR; //因為analogWrite只吃正數，所以如果本來是負數，就要乘-1
  }
  if (vL >= 0) {
    digitalWrite(MotorL_I1, LOW);
    digitalWrite(MotorL_I2, HIGH);
    //這邊的Motor第幾個對應到的High/Low是助教的車對應到的，請自己測試自己車該怎麼填！
  } else if (vL < 0) {
    digitalWrite(MotorL_I1, HIGH);
    digitalWrite(MotorL_I2, LOW);
    vL = -vL; //因為analogWrite只吃正數，所以如果本來是負數，就要乘-1
  }
  analogWrite(MotorL_PWML, vL);
  analogWrite(MotorR_PWMR, vR);
}



void Tracking(int deltaTime) {
  if (stop) {
    MotorWriting(0, 0);
    return;
  }
  int IR[5];
  for (int i = 0 ; i < analognum ; i++) {
    int sensorValue = analogRead(analogPin[i]); // 宣告 sensorValue 這變數是整數(Integer)
    IR[i] = sensorValue;
    if (IR[i] > 100) IR[i] = 1;
    else IR[i] = 0;
    //Serial.println(IR[i]); // 將數值印出來
  }
  if (turning) {
    double turnspeed = 50;
    if (dir == 0)
      MotorWriting(-turnspeed, turnspeed);
    else if (dir == 1 || dir == 3)
      MotorWriting(turnspeed, -turnspeed);
      
    turntime += deltaTime;
    if (dir == 2 && IR[0] == 0 && IR[4] == 0) {
      turning = 0;
      innode = 0;
    }
    if ((dir == 0 || dir == 1) && (turntime > 600 && IR[2] == 1) ) {
      //MotorWriting(0, 0);
      turning = 0;
      innode = 0;
    }
    if ((dir == 3) && (turntime > 1000 && IR[2] == 1) ) {
      //MotorWriting(0, 0);
      turning = 0;
      innode = 0;
    }
  }
  if (!turning) {
    double Kprate = 0.1, Tp = 100;
    double w3 = 5, w2 = 3;
    double error = 0;
    if (IR[0] + IR[1] + IR[3] + IR[4] > 0)
      error = (IR[0]*(-w3) + IR[1]*(-w2) + IR[3]*w2 + IR[4]*w3) / (IR[0] +IR[1] + IR[2] + IR[3] + IR[4]);
    int powerCorrection = Kprate * Tp * error; // ex. Kp = 100, 也與w2 & w3有關
    int vR = Tp - powerCorrection; // ex. Tp = 150, 也與w2 & w3有關
    int vL = Tp + powerCorrection;
    if(vR>255) vR = 255;
    if(vL>255) vL = 255;
    if(vR<-255) vR = -255;
    if(vL<-255) vL = -255;
    int sum = IR[0] + IR[1] + IR[2] + IR[3] + IR[4];
    if (sum >= 4) innode = 1;
    if (innode && sum <= 2) {
      turning = 1;
      turntime = 0;
      dir = mode[state++];
      if (dir == -1) stop = 1;
    }
    
    MotorWriting(vL, vR); //Feedback to motors
    lastsum = sum;
  }


  
}

void setup() {
  Serial.begin(9600); // 表示開始傳遞與接收序列埠資料
  initIR();
  //initRFID();
  initMotor();
}

void loop() {
  unsigned long currentTime = millis();

  // 只有時間到了，才執行 Tracking
  if (currentTime >= nextLoopTime) {
    int deltatime = currentTime - lastLoopstartTime;
    lastLoopstartTime = currentTime;
    
    // 設定下一次執行目標時間
    nextLoopTime = currentTime + targetLoopTime;

    Tracking(deltatime);

    /*MotorWriting(210, 200);
    delay(5000);
    MotorWriting(210, -200);
    delay(2000);
    MotorWriting(-210, -200);
    delay(5000);*/

    //testIR();
    //testRFID();
    //testMotor();
    
    // 把數據傳給 Python
    
  }

  // 藍牙指令
  if (Serial.available() > 0) {
    // Read "s"
  }
}