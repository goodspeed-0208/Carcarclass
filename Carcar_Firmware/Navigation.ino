
void CarCar::navigating(int deltaTime) {
	int lastMotor_vL = motor_vL; //判斷要不要MotorWriting();
	int lastMotor_vR = motor_vR;
	if (!isRunning) {
		motor_vL = 0, motor_vR = 0;
	} else if (turning) {
		if (dir == LEFT)
			motor_vL = -turnspeed, motor_vR = turnspeed;
		else if (dir == RIGHT || dir == TURN_BACK)
			motor_vL = turnspeed, motor_vR = -turnspeed;
		else if (dir == FORWARD)
			motor_vL = forwardspeed, motor_vR = forwardspeed;

		turntime += deltaTime;
		if (dir == FORWARD && IRisBlack[0] == 0 && IRisBlack[4] == 0) {
			turning = 0;
			isInnode = 0;
		}
		if ((dir == LEFT || dir == RIGHT) && (turntime > 300 && (IRisBlack[2] == 1 || IRisBlack[1] == 1 || IRisBlack[3] == 1))) {
			//MotorWriting(0, 0);
			turning = 0;
			isInnode = 0;
		}
		if ((dir == TURN_BACK) && (turntime > 500 && (IRisBlack[2] == 1 || IRisBlack[1] == 1 || IRisBlack[3] == 1))) {
			//MotorWriting(0, 0);
			turning = 0;
			isInnode = 0;
		}
	} else if (!turning){
		Tracking(deltaTime);
	}
	if(lastMotor_vL != motor_vL || lastMotor_vR != motor_vR){ //判斷要不要MotorWriting();
		MotorWriting();
	}
}

void CarCar::Tracking(int deltaTime) {
	double Kprate = 0.1, Tp = 100;
	double w3 = 5, w2 = 3;
	double error = 0;
	if (IRisBlack[0] + IRisBlack[1] + IRisBlack[3] + IRisBlack[4] > 0)
		error = (IRisBlack[0] * (-w3) + IRisBlack[1] * (-w2) + IRisBlack[3] * w2 + IRisBlack[4] * w3) / (IRisBlack[0] + IRisBlack[1] + IRisBlack[2] + IRisBlack[3] + IRisBlack[4]);
	int powerCorrection = Kprate * Tp * error;  // ex. Kp = 100, 也與w2 & w3有關  //沒看懂等改
	int vR = Tp - powerCorrection;              // ex. Tp = 150, 也與w2 & w3有關
	int vL = Tp + powerCorrection;
	if (vR >= 255) vR = 255;
	if (vL >= 255) vL = 255;
	if (vR <= -255) vR = -255;
	if (vL <= -255) vL = -255;
	int sum = IRisBlack[0] + IRisBlack[1] + IRisBlack[2] + IRisBlack[3] + IRisBlack[4];
	if (sum >= 4) {
		isInnode = 1;
		//Serial3.print("in_node");
	}
	if (isInnode && sum <= 2) {
		turning = 1;
		turntime = 0;
		dir = mode[modeState];
		modeState = (modeState + 1) % 8;
		if (dir == STAY_STOP) isRunning = 0;
	}

	motor_vL = vL, motor_vR = vR;  //Feedback to CarCar
}

void CarCar::stop(){
	isRunning = 0;
}

void CarCar::restart(){
  isInnode = 0;
  turntime = 0;
  dir = mode[modeState];
  turning = 0;
  isRunning = 1;
  modeState = 0;
}