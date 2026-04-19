
void CarCar::navigating(int deltaTime) {

	if (!isRunning) {
		target_motor_vL = 0;
		target_motor_vR = 0;
		MotorWriting(deltaTime);
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
			else if (dir == LEFT_AFTER_BACKWARD) turnleft_after_backward();
			else if (dir == RIGHT_AFTER_BACKWARD) turnright_after_backward();
			else if (dir == STAY_STOP) stop();

			turntime += deltaTime;
			if (!turning) {
				Tracking(0);  // Pass 0 as deltaTime since time was consumed by turning
			}
		}

	MotorWriting(deltaTime);

	if(!turning || (turning && dir == FORWARD)){ //紀錄直行平均速度
			trackCount++;
			sum_vL += last_motor_vL;
			sum_vR += last_motor_vR;
			averagevL = sum_vL/trackCount;
			averagevR = sum_vR/trackCount;
		}
		else if(turning && dir != BACKWARD){
			sum_vL = 0;
    	sum_vR = 0;
    	trackCount = 0;
		}
	}
}

void CarCar::goForward() {
	target_motor_vL = forwardspeed, target_motor_vR = forwardspeed;
	if (IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
		Serial3.println("outnode");
	}
}

void CarCar::turnleft() {
	target_motor_vL = turnInnerSpeed, target_motor_vR = turnOuterSpeed;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
		Serial3.println("outnode");
	}
}

void CarCar::turnright() {
	target_motor_vL = turnOuterSpeed, target_motor_vR = turnInnerSpeed;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
		Serial3.println("outnode");
	}
}

void CarCar::turnback() {
	target_motor_vL = turnBackSpeed, target_motor_vR = -turnBackSpeed;
	if (turntime >= Min_turnback_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
		Serial3.println("outnode");
	}
}

void CarCar::goBackward() {  //turning持續到回到上一個節點，目前功能尚不齊全
	target_motor_vL = -averagevL, target_motor_vR = -averagevR;
	if (turntime >= Min_backward_turntime && IRsum >= 4) {
		turning = 0;
		isInnode = 1;
		modeState = (modeState + 1) % 8;
		sum_vL = 0;
    sum_vR = 0;
    trackCount = 0;
		Serial3.println("outnode");
	}
}

void CarCar::turnleft_after_backward() {
	target_motor_vL = turnInnerSpeed_back, target_motor_vR = turnOuterSpeed_back;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
		Serial3.println("outnode");
	}
}

void CarCar::turnright_after_backward() {
	target_motor_vL = turnOuterSpeed_back, target_motor_vR = turnInnerSpeed_back;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
		Serial3.println("outnode");
	}
}

void CarCar::Tracking(int deltaTime) {
	double w3 = 5, w2 = 3;
	double error = 0;
	if (IRisBlack[0] + IRisBlack[1] + IRisBlack[3] + IRisBlack[4] > 0)
		error = (IRisBlack[0] * (-w3) + IRisBlack[1] * (-w2) + IRisBlack[3] * w2 + IRisBlack[4] * w3) / IRsum;
	double dt = deltaTime;
	if (dt <= 0) {
		dt = 50;
		lastError = error;
	}
	integral += error * dt;
	double derivative = (error - lastError) / dt;
	double correction = Kp * error + Ki * integral + Kd * derivative;
	int powerCorrection = (int)(correction * forwardspeed);
	int vR = forwardspeed - powerCorrection;              // ex. Tp = 150, 也與w2 & w3有關
	int vL = forwardspeed + powerCorrection;
	if (vR >= 255) vR = 255;
	if (vL >= 255) vL = 255;
	if (vR <= -255) vR = -255;
	if (vL <= -255) vL = -255;
	if (IRsum >= 4) {
		isInnode = 1;
		turning = 1;
		turntime = 0;
		integral = 0;
    lastError = 0;

		dir = next_dir;
		Serial3.println("innode");
		//Serial3.print("this node:");
		Serial3.println(dir);
	}
	else {
    lastError = error;
  }

	/*if (isInnode && IRsum <= 2) {
		turning = 1;
		turntime = 0;
		dir = mode[modeState];
		if (dir == STAY_STOP) isRunning = 0;
	}*/

	target_motor_vL = vL, target_motor_vR = vR;  //Feedback to CarCar

	//MotorWriting(deltaTime); //記得刪
}

void CarCar::stop() {
	isRunning = 0;
}

void CarCar::restart() {
	isInnode = 0;
	turntime = 0;
	modeState = 0;
	turning = 0;
	isRunning = 1;
	lastError = 0;
  integral = 0;
}

void CarCar::adjust_motor_error(int deltatime) { //根據目前的速度與差值走直線，不做tracking
	if(!isRunning) {
		target_motor_vL = 0;
		target_motor_vR = 0;
	}
	else{
		target_motor_vL = forwardspeed;
		target_motor_vR = forwardspeed;
		if(IRsum>=4) {
			int current_adjust_time = millis() - start_time;
			if(!adjust_start){
				start_time = millis();
				adjust_start = 1;
			}
			else if(current_adjust_time >= 1500) {
				isRunning = 0;
				adjust_start = 0;
				Serial3.println(current_adjust_time);
			}
			
		}
	}

		MotorWriting(deltatime);
}

void CarCar::start_turn_test(char dir, int outer, float ratio) {
	testDir = dir;
	testOuterSpeed = outer;
	testInnerSpeed = outer * ratio; // Calculate inner speed safely
	testState = TEST_TRACK;         // Enter tracking state
	isRunning = 1;
	
	Serial.println("Test Started. Tracking to next node...");
	Serial3.println("Test Started. Tracking to next node...");
}

void CarCar::run_turn_test(int deltaTime) {
	if (!isRunning) {
		target_motor_vL = 0;
		target_motor_vR = 0;
		MotorWriting(deltaTime);
		return;
	}

	switch (testState) {
		case TEST_WAIT:
			target_motor_vL = 0;
			target_motor_vR = 0;
			break;

		case TEST_TRACK:
			// 1. Track the line to ensure perfect alignment before the turn
			Tracking(deltaTime); 
			
			// 2. Check if we hit the node
			if (IRsum >= 4) {    
				testState = TEST_TURN;
				turnStartTime = millis();
				Serial3.println("Node detected! Starting turn...");
			}
			break;

		case TEST_TURN:
			// 1. Set turning speeds
			if (testDir == 'L') {
				target_motor_vL = testInnerSpeed;
				target_motor_vR = testOuterSpeed;
			} else if (testDir == 'R') {
				target_motor_vL = testOuterSpeed;
				target_motor_vR = testInnerSpeed;
			}

			int currentTurnTime = millis() - turnStartTime;
			int deadzoneTime = 70000/testOuterSpeed; // Blind turning time to escape the node (adjust if needed)

			// 2. Exit condition: Passed deadzone AND center IR sees the line
			if (currentTurnTime > deadzoneTime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
				testState = TEST_WAIT;
				isRunning = 0; // Emergency stop!
				target_motor_vL = 0;
				target_motor_vR = 0;

				Serial3.print("Turn Finished! Time taken (ms): ");
				Serial3.println(currentTurnTime);
			}
			break;
	}

	// Always execute motor writing to maintain Slew Rate Limiting
	MotorWriting(deltaTime);
}
