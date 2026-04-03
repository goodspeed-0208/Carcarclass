
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
  }  //PICC_IsNewCardPresent()：是否感應到新的卡片?
  if (!mfrc522->PICC_ReadCardSerial()) {
    return;
  }  //PICC_ReadCardSerial()：是否成功讀取資料?
  Serial.println(F("**Card Detected:**"));

  Serial3.println(F("**Card Detected:**"));
  Serial3.print(F("UID: "));
  for (byte i = 0; i < mfrc522->uid.size; i++) {
    Serial3.print(mfrc522->uid.uidByte[i], HEX);
    Serial3.print(" ");
  }
  Serial3.println();
  mfrc522->PICC_DumpDetailsToSerial(&(mfrc522->uid));  //讀出 UID
  //mfrc522->PICC_DumpDetailsToSerial3(&(mfrc522->uid)); //讀出 UID
  mfrc522->PICC_HaltA();       // 讓同一張卡片進入停止模式 (只顯示一次)
  mfrc522->PCD_StopCrypto1();  // 停止 Crypto1
}