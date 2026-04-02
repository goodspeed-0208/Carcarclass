void Tracking(int deltaTime) {
  if (stop) {
    MotorWriting(0, 0);
    return;
  }
  int IR[5];
  for (int i = 0 ; i < analognum ; i++) {
    int sensorValue = analogRead(analogPin[i]); // 宣告 sensorValue 這變數是整數(Integer)
    IR[i] = sensorValue;
    if (tracktime >= 10) {
      Serial3.print(IR[i]);
      Serial3.print(" ");
    }
    
    if (IR[i] > 40) IR[i] = 1;
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
    if ((dir == 0 || dir == 1) && (turntime > 300 && (IR[2] == 1 || IR[1] == 1 || IR[3] == 1) ) ) {
      //MotorWriting(0, 0);
      turning = 0;
      innode = 0;
    }
    if ((dir == 3) && (turntime > 500 && (IR[2] == 1 || IR[1] == 1 || IR[3] == 1)) ) {
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
    if (sum >= 4) {
      innode = 1;
      //Serial3.print("in_node");
    }
    if (innode && sum <= 2) {
      turning = 1;
      turntime = 0;
      dir = mode[state];
      state = (state + 1) % 8;
      if (dir == -1) stop = 1;
    }
    
    MotorWriting(vL, vR); //Feedback to motors
    lastsum = sum;
  }

}