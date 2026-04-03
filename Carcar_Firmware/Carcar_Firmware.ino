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

class CarCar{
  public:
    CarCar();
    ~CarCar();

		void begin();

    void Tracking(int deltaTime);
    void readIR();
    void readRFID();

    void stop(); //目前放在Tracking
    void restart();

  private:
		void initIR();
		void initRFID();
		void initMotor();
    void MotorWriting(double vL, double vR);

  private:
    int MotorR_I3 = BIN1, MotorR_I4 = BIN2, MotorL_I1 = AIN1, MotorL_I2 = AIN2;
    int MotorL_PWML = PWMA, MotorR_PWMR = PWMB;

		int RFID_SS_PIN = SS_PIN;
    int RFID_RST_PIN = RST_PIN;
    MFRC522 *mfrc522;


    //Navigation(關注在前進左右倒退迴轉的模式)
    bool running = 0;
    bool isInode = 0;
    bool turning = 0;
    int turntime = 0;
    int dir; // left right forward baackward
    int mode[8] = { 1, 3, 2, 3, 0, 3, 2, 3 }; 
    int modeState = 0;
    
    //Tracking(關注在前進(或後退)的循跡演算法)
    

    //IR
    int IRvalue[analognum] = {0, 0, 0, 0, 0};
    int lastIRsum = 0;
    int IRtracktime = 0;

    // BLUE TOOTH(class CarCar裡不會有bluetooth，因為bluetooth是車車與電腦的溝通橋樑)

    

};

CarCar mycar;

unsigned long sendTime;

void setup() {
  //Serial.begin(9600); // 表示開始傳遞與接收序列埠資料
  //Serial.begin(9600);
  initBlueTooth();
	mycar.begin();
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

		mycar.Tracking(deltatime);
		mycar.readRFID();
		//readIR();
  }

  if (Serial3.available()) {
		String command = Serial3.readStringUntil('\n'); //此處原本為String command = Serial3.readString();
    command.trim();
		Serial.println(command);
		if (command == "receive init") {
			Serial.print(millis() - sendTime);
			Serial3.print(millis() - sendTime);
		}
		if (command == "e") mycar.stop();  // end
		else if (command == "s") {     // start
			mycar.restart();
		}
  }
}
