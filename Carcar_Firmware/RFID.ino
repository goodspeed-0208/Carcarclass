
void CarCar::initRFID() {

  SPI.begin();
  mfrc522 = new MFRC522(RFID_SS_PIN, RFID_RST_PIN);
  // 請系統去要一塊記憶體空間，後面呼叫它的建構函式
  // 將(SS, RST) 當成參數傳進去初始化。
  mfrc522->PCD_Init();
  /* 初始化MFRC522讀卡機 PCD_Init 模組。 -> 表示：
    透過記憶體位置，找到 mfrc522 這物件，再翻其內容。*/
  Serial.println(F("Read UID on a MIFARE PICC:"));
}

void CarCar::readRFID() {
  if (!mfrc522->PICC_IsNewCardPresent()) {
    return;
  }
  if (!mfrc522->PICC_ReadCardSerial()) {
    return;
  }

  // Print full details to the USB Serial monitor for local debugging
  //Serial.println(F("**Card Detected:**"));
  mfrc522->PICC_DumpDetailsToSerial(&(mfrc522->uid));

  // Pack the UID into a single string for Bluetooth transmission
  String uidString = "uid:";

  for (byte i = 0; i < mfrc522->uid.size; i++) {
    // Add a leading zero if the hex value is less than 0x10 (16)
    if (mfrc522->uid.uidByte[i] < 0x10) {
      uidString += "0";
    }
    uidString += String(mfrc522->uid.uidByte[i], HEX);
  }

  // Send the complete string in one packet to avoid BLE fragmentation

  // 2. 新增：檢查是否已經讀取過
  for (int i = 0; i < visitedCount; i++) {
    if (visitedUIDs[i] == uidString) {
      // 已經讀過這張卡了，直接停止後續動作
      mfrc522->PICC_HaltA();
      mfrc522->PCD_StopCrypto1();
      return; 
    }
  }

  // 3. 新增：若是新卡，加入紀錄
  if (visitedCount < MAX_VISITED) {
    visitedUIDs[visitedCount] = uidString;
    visitedCount++;
  }

  isFindingRFID = false;


  if (turning || !isRunning) {
    // IR triggered first. Do not overwrite dir, next_dir, or turntime.
    // Only send the RFID data via Bluetooth to notify the system.
    char btBuffer[80];
    snprintf(btBuffer, sizeof(btBuffer), "%s", uidString.c_str());
    Serial3.println(btBuffer);
  } else {
    // Normal case: RFID triggered first or perfectly simultaneous with IR.
    isInnode = 1;  
    turning = 1;
    turntime = 0;
    integral = 0;
    lastError = 0;

    unsigned long curTime = millis();
    unsigned long motion_duration = curTime - motion_startTime;
    
    dir = next_dir;
    next_dir = WAIT_FOR_COMMAND;

    char btBuffer[80];
    snprintf(btBuffer, sizeof(btBuffer), "%s,Tr:%lu,INN,dir:%s",
             uidString.c_str(), motion_duration, getDirString(dir).c_str());
    Serial3.println(btBuffer);

    currentSegmentType = 4;
    trackingData[currentSegmentType].update(motion_duration);

    motion_startTime = curTime;
  }

  // Halt PICC and stop encryption on PCD
  mfrc522->PICC_HaltA();
  mfrc522->PCD_StopCrypto1();
}