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
int delaytime = 20;
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
  delay (500); // 延遲 2 秒
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

void initBlueTooth() {
  Serial.begin(115200); // Debug Monitor (USB)
  while (!Serial);
  Serial.println("Initializing HM-10...");

  // 1. Automatic Baud Rate Detection
  for (int i = 0; i < 9; i++) {
    Serial.print("Testing baud rate: ");
    Serial.println(baudRates[i]);
    
    Serial3.begin(baudRates[i]);
    Serial3.setTimeout(100);

    // 2. Force Disconnection
    // Sending "AT" while connected forces the module to disconnect [2].
    Serial3.print("AT"); 
    
    if (waitForResponse("OK", 800)) {
      Serial.println("HM-10 detected and ready.");
      moduleReady = true;
      break; 
    } else {
      Serial3.end();
    }
  }

  if (!moduleReady) {
    Serial.println("Failed to detect HM-10. Check 3.3V VCC and wiring.");
    return;
  }

  // 3. Restore Factory Defaults
  Serial.println("Restoring factory defaults...");
  sendATCommand("AT+RENEW"); // Restores all setup values

  // 4. Set Custom Name via Macro
  Serial.print("Setting name to: ");
  Serial.println(CUSTOM_NAME);
  String nameCmd = "AT+NAME" + String(CUSTOM_NAME);
  sendATCommand(nameCmd.c_str()); // Max length is 12
  
  // 5. Enable Connection Notifications
  Serial.println("Enabling notifications...");
  sendATCommand("AT+NOTI1"); // Notify when link is established/lost

  // 6. Get the Bluetooth MAC Address
  Serial.println("Querying Bluetooth Address");
  sendATCommand("AT+ADDR?");

  // 7. Restart the module to apply changes
  Serial.println("Restarting module...");
  sendATCommand("AT+RESET"); // Restart the module
  Serial3.begin(9600); // Now the module would use baudrate 9600
  
  Serial.println("Initialization Complete.");
}

void testRFID() {
  if(!mfrc522->PICC_IsNewCardPresent()) {
    goto FuncEnd;
  } //PICC_IsNewCardPresent()：是否感應到新的卡片?
  if(!mfrc522->PICC_ReadCardSerial()) {
    goto FuncEnd;
  } //PICC_ReadCardSerial()：是否成功讀取資料?
  Serial.println(F("**Card Detected:**"));
  
  Serial3.println(F("**Card Detected:**"));
  Serial3.print(F("UID: "));
  for (byte i = 0; i < mfrc522 -> uid.size; i++) {
      Serial3.print(mfrc522 -> uid.uidByte[i], HEX);
      Serial3.print(" ");
  }
  Serial3.println();
  mfrc522->PICC_DumpDetailsToSerial(&(mfrc522->uid)); //讀出 UID
  //mfrc522->PICC_DumpDetailsToSerial3(&(mfrc522->uid)); //讀出 UID
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

void Tracking() {
  if (stop) {
    MotorWriting(0, 0);
    return;
  }
  int IR[5];
  tracktime++;
  for (int i = 0 ; i < analognum ; i++) {
    int sensorValue = analogRead(analogPin[i]); // 宣告 sensorValue 這變數是整數(Integer)
    IR[i] = sensorValue;
    if (tracktime >= 10) {
      Serial3.print(IR[i]);
      Serial3.print(" ");
    }
    
    if (IR[i] > 100) IR[i] = 1;
    else IR[i] = 0;
    //Serial.println(IR[i]); // 將數值印出來
  }
  if (tracktime >= 500/delaytime) {
    Serial3.println();
    tracktime = 0;
  }
  if (turning) {
    double turnspeed = 50;
    if (dir == 0)
      MotorWriting(-turnspeed, turnspeed);
    else if (dir == 1 || dir == 3)
      MotorWriting(turnspeed, -turnspeed);
      
    turntime+=delaytime;
    if (dir == 2 && IR[0] == 0 && IR[4] == 0) {
      turning = 0;
      innode = 0;
    }
    if ((dir == 0 || dir == 1) && (turntime > 300 && (IR[2] == 1 || IR[1] == 1 || IR[3] == 1) ) ) {
      //MotorWriting(0, 0);
      turning = 0;
      innode = 0;
    }
    if ((dir == 3) && (turntime > 600 && (IR[2] == 1 || IR[1] == 1 || IR[3] == 1)) ) {
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
      state %= 8;
      if (dir == -1) stop = 1;
    }
    
    MotorWriting(vL, vR); //Feedback to motors
    lastsum = sum;
  }


  
}

void setup() {

  initVar();
  //Serial.begin(9600); // 表示開始傳遞與接收序列埠資料
  initIR();
  initRFID();
  initMotor();
  //Serial.begin(9600);
  initBlueTooth();
}

void loop(){
  //testIR();
  Tracking();
  testRFID();
  delay(delaytime);

  if (Serial3.available()) {
    String command = Serial3.readString();
    Serial.println(command);
    if(command == "e") stop = 1;
    else if (command == "s") {
      initVar();
    }
  }
  

  //testIR();
  //testMotor();
}

void sendATCommand(const char* command) {
  Serial3.print(command);
  waitForResponse("", 1000); 
}

/**
 * Helper to check response for specific substrings
 */
bool waitForResponse(const char* expected, unsigned long timeout) {
  unsigned long start = millis();
  Serial3.setTimeout(timeout);
  String response = Serial3.readString();
  if (response.length() > 0) {
    Serial.print("HM10 Response: ");
    Serial.println(response);
  }
  return (response.indexOf(expected) != -1);
}