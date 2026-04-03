
void CarCar::initIR() {
  for (int i = 0; i < analognum; i++) pinMode(analogPin[i], INPUT);  // 目前預設該接腳作為輸入
}

void CarCar::readIR() {
  String msg = "";
  for (int i = 0; i < analognum; i++) {
    int sensorValue = analogRead(analogPin[i]);  // 宣告 sensorValue 這變數是整數(Integer)
    IRvalue[i] = sensorValue;
    Serial.println(sensorValue);  // 將數值印出來
    msg = msg + IRvalue[i] + " ";
  }
  Serial3.println(msg);
  //Serial3.println(r[0] + " " + r[1] + " " + r[2] + " " + r[3] + " " + r[4]);
  Serial.println("__________________");
  delay(500);  // 延遲 2 秒
}