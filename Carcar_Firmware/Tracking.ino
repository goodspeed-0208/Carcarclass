
void CarCar::Tracking(int deltaTime) {
	if (!running) {
		MotorWriting(0, 0);
		return;
	}
	for (int i = 0; i < analognum; i++) {
		int sensorValue = analogRead(analogPin[i]);  // 宣告 sensorValue 這變數是整數(Integer)
		IRvalue[i] = sensorValue;
		if (IRtracktime >= 10) {
			Serial3.print(IRvalue[i]);
			Serial3.print(" ");
		}

		if (IRvalue[i] > 40) IRvalue[i] = 1;
		else IRvalue[i] = 0;
		//Serial.println(IRvalue[i]); // 將數值印出來
	}

	if (turning) {
		double turnspeed = 50;
		if (dir == 0)
			MotorWriting(-turnspeed, turnspeed);
		else if (dir == 1 || dir == 3)
			MotorWriting(turnspeed, -turnspeed);

		turntime += deltaTime;
		if (dir == 2 && IRvalue[0] == 0 && IRvalue[4] == 0) {
			turning = 0;
			isInode = 0;
		}
		if ((dir == 0 || dir == 1) && (turntime > 300 && (IRvalue[2] == 1 || IRvalue[1] == 1 || IRvalue[3] == 1))) {
			//MotorWriting(0, 0);
			turning = 0;
			isInode = 0;
		}
		if ((dir == 3) && (turntime > 500 && (IRvalue[2] == 1 || IRvalue[1] == 1 || IRvalue[3] == 1))) {
			//MotorWriting(0, 0);
			turning = 0;
			isInode = 0;
		}
	}
	if (!turning) {
		double Kprate = 0.1, Tp = 100;
		double w3 = 5, w2 = 3;
		double error = 0;
		if (IRvalue[0] + IRvalue[1] + IRvalue[3] + IRvalue[4] > 0)
			error = (IRvalue[0] * (-w3) + IRvalue[1] * (-w2) + IRvalue[3] * w2 + IRvalue[4] * w3) / (IRvalue[0] + IRvalue[1] + IRvalue[2] + IRvalue[3] + IRvalue[4]);
		int powerCorrection = Kprate * Tp * error;  // ex. Kp = 100, 也與w2 & w3有關
		int vR = Tp - powerCorrection;              // ex. Tp = 150, 也與w2 & w3有關
		int vL = Tp + powerCorrection;
		if (vR > 255) vR = 255;
		if (vL > 255) vL = 255;
		if (vR < -255) vR = -255;
		if (vL < -255) vL = -255;
		int sum = IRvalue[0] + IRvalue[1] + IRvalue[2] + IRvalue[3] + IRvalue[4];
		if (sum >= 4) {
			isInode = 1;
			//Serial3.print("in_node");
		}
		if (isInode && sum <= 2) {
			turning = 1;
			turntime = 0;
			dir = mode[modeState];
			modeState = (modeState + 1) % 8;
			if (dir == -1) running = 0;
		}

		MotorWriting(vL, vR);  //Feedback to motors
		lastIRsum = sum;
	}
}

void CarCar::stop(){
	running = 0;
}

void CarCar::restart(){
	lastIRsum = 0;
  isInode = 0;
  turntime = 0;
  dir = 3;
  turning = 0;
  running = 1;
  modeState = 0;
}