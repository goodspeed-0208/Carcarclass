
void CarCar::navigating(int deltaTime) {
	int lastMotor_vL = motor_vL;  //判斷要不要MotorWriting();
	int lastMotor_vR = motor_vR;

	if (!isRunning) {
		motor_vL = 0, motor_vR = 0;
	} else {
		if (!turning) {
			Tracking(deltaTime);
		}
		if (turning) {
			if (dir == STAY_STOP) isRunning = 0;
			else if (dir == LEFT) turnleft();
			else if (dir == RIGHT) turnright();
			else if (dir == FORWARD) goForward();
			else if (dir == TURN_BACK) turnback();
			else if (dir == BACKWARD) goBackward();
			turntime += deltaTime;
			if (!turning) {
				Tracking(0);  // Pass 0 as deltaTime since time was consumed by turning
			}
		}
	}

	if (lastMotor_vL != motor_vL || lastMotor_vR != motor_vR) {  //判斷要不要MotorWriting();
		MotorWriting();
	}
}

void CarCar::goForward() {
	motor_vL = forwardspeed, motor_vR = forwardspeed;
	if (IRisBlack[0] == 0 && IRisBlack[4] == 0) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
	}
}

void CarCar::turnleft() {
	motor_vL = turnInnerSpeed, motor_vR = turnOuterSpeed;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
	}
}

void CarCar::turnright() {
	motor_vL = turnOuterSpeed, motor_vR = turnInnerSpeed;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
	}
}

void CarCar::turnback() {
	motor_vL = turnBackSpeed, motor_vR = -turnBackSpeed;
	if (turntime >= Min_turnback_turntime && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
	}
}

void CarCar::goBackward() {  //turning持續到回到上一個節點，目前功能尚不齊全
	motor_vL = -backwardspeed, motor_vR = -backwardspeed;
	if (turntime >= Min_backward_turntime && IRsum >= 4) {
		turning = 0;
		isInnode = 1;
		modeState = (modeState + 1) % 8;
	}
}

void CarCar::Tracking(int deltaTime) {
	double Kprate = 0.1, Tp = 100;
	double w3 = 5, w2 = 3;
	double error = 0;
	if (IRisBlack[0] + IRisBlack[1] + IRisBlack[3] + IRisBlack[4] > 0)
		error = (IRisBlack[0] * (-w3) + IRisBlack[1] * (-w2) + IRisBlack[3] * w2 + IRisBlack[4] * w3) / IRsum;
	int powerCorrection = Kprate * Tp * error;  // ex. Kp = 100, 也與w2 & w3有關  //沒看懂等改
	int vR = Tp - powerCorrection;              // ex. Tp = 150, 也與w2 & w3有關
	int vL = Tp + powerCorrection;
	if (vR >= 255) vR = 255;
	if (vL >= 255) vL = 255;
	if (vR <= -255) vR = -255;
	if (vL <= -255) vL = -255;
	if (IRsum >= 4) {
		isInnode = 1;
		turning = 1;
		turntime = 0;
		dir = mode[modeState];
		//Serial3.print("in_node");
	}
	/*if (isInnode && IRsum <= 2) {
		turning = 1;
		turntime = 0;
		dir = mode[modeState];
		if (dir == STAY_STOP) isRunning = 0;
	}*/

	motor_vL = vL, motor_vR = vR;  //Feedback to CarCar
}

void CarCar::stop() {
	isRunning = 0;
}

void CarCar::restart() {
	isInnode = 0;
	turntime = 0;
	modeState = 0;
	dir = mode[modeState];
	turning = 0;
	isRunning = 1;
}