
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
			if (dir == STAY_STOP) {
				if (isRunning) {
					stop(deltaTime);
					reportData();
				}
			} else if (dir == LEFT) turnleft();
			else if (dir == RIGHT) turnright();
			else if (dir == FORWARD) goForward();
			else if (dir == TURN_BACK) turnback();
			else if (dir == BACKWARD) goBackward();
			else if (dir == LEFT_AFTER_BACKWARD) turnleft_after_backward();
			else if (dir == RIGHT_AFTER_BACKWARD) turnright_after_backward();
			else if (dir == WAIT_FOR_COMMAND) wait_for_command();

			turntime += deltaTime;
			if (!turning) {
				Tracking(0);  // Pass 0 as deltaTime since time was consumed by turning
			}
		}

		MotorWriting(deltaTime);

		if (!turning || (turning && dir == FORWARD)) {  //紀錄直行平均速度
			trackCount++;
			sum_vL += last_motor_vL;
			sum_vR += last_motor_vR;
			averagevL = sum_vL / trackCount;
			averagevR = sum_vR / trackCount;
		} else if (turning && dir != BACKWARD) {
			sum_vL = 0;
			sum_vR = 0;
			trackCount = 0;
		}
	}
}

void CarCar::goForward() {
	target_motor_vL = forwardspeed, target_motor_vR = forwardspeed;
	if (IRsum <= 2 && IRsum >= 1) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		String btMsg = getDirString(dir) + ":" + String(motion_duration) + "\n";
		btMsg += "runTime:" + String(curTime - start_time) + "\n";
		btMsg += "outn";
		Serial3.println(btMsg);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
		next_dir = WAIT_FOR_COMMAND;
	}
}

void CarCar::turnleft() {
	target_motor_vL = turnInnerSpeed, target_motor_vR = turnOuterSpeed;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		String btMsg = getDirString(dir) + ":" + String(motion_duration) + "\n";
		btMsg += "runTime:" + String(curTime - start_time) + "\n";
		btMsg += "outn";
		Serial3.println(btMsg);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
		next_dir = WAIT_FOR_COMMAND;
	}
}

void CarCar::turnright() {
	target_motor_vL = turnOuterSpeed, target_motor_vR = turnInnerSpeed;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		String btMsg = getDirString(dir) + ":" + String(motion_duration) + "\n";
		btMsg += "runTime:" + String(curTime - start_time) + "\n";
		btMsg += "outn";
		Serial3.println(btMsg);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
		next_dir = WAIT_FOR_COMMAND;
	}
}

void CarCar::turnback() {
	target_motor_vL = turnBackSpeed, target_motor_vR = -turnBackSpeed;
	if (turntime >= Min_turnback_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		String btMsg = getDirString(dir) + ":" + String(motion_duration) + "\n";
		btMsg += "runTime:" + String(curTime - start_time) + "\n";
		btMsg += "outn";
		Serial3.println(btMsg);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
		next_dir = WAIT_FOR_COMMAND;
	}
}

void CarCar::goBackward() {  //turning持續到回到上一個節點，目前功能尚不齊全
	target_motor_vL = -averagevL, target_motor_vR = -averagevR;
	if(isInnode && IRsum <= 2){
		isInnode = 0;
		String btMsg = "outn";
		Serial3.println(btMsg);
	}
	if (turntime >= Min_backward_turntime && !isInnode && IRsum >= 4) {
		turning = 1;
		isInnode = 1;
		modeState = (modeState + 1) % 8;
		sum_vL = 0;
		sum_vR = 0;
		trackCount = 0;
		
		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		String btMsg = getDirString(dir) + ":" + String(motion_duration) + "\n";
		btMsg += "runTime:" + String(curTime - start_time) + "\n";
		turningData[dir].update(motion_duration);

		dir = next_dir;
		next_dir = WAIT_FOR_COMMAND;
		btMsg += "inn\ndir:" + getDirString(dir);
		Serial3.println(btMsg);

		motion_startTime = curTime;
	}
}

void CarCar::turnleft_after_backward() {
	target_motor_vL = turnOuterSpeed_back, target_motor_vR = turnInnerSpeed_back;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		String btMsg = getDirString(dir) + ":" + String(motion_duration) + "\n";
		btMsg += "runTime:" + String(curTime - start_time) + "\n";
		btMsg += "outn";
		Serial3.println(btMsg);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
		next_dir = WAIT_FOR_COMMAND;
	}
}

void CarCar::turnright_after_backward() {
	target_motor_vL = turnInnerSpeed_back, target_motor_vR = turnOuterSpeed_back;
	if (turntime >= Min_rightleft_turntime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
		turning = 0;
		isInnode = 0;
		modeState = (modeState + 1) % 8;
		
		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		String btMsg = getDirString(dir) + ":" + String(motion_duration) + "\n";
		btMsg += "runTime:" + String(curTime - start_time) + "\n";
		btMsg += "outn";
		Serial3.println(btMsg);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
		next_dir = WAIT_FOR_COMMAND;
	}
}

void CarCar::wait_for_command() {
	target_motor_vL = 0, target_motor_vR = 0;
	
	if(next_dir != WAIT_FOR_COMMAND) {
		dir = next_dir;
		next_dir = WAIT_FOR_COMMAND;
		turntime = 0;
		motion_startTime = millis();
	}
}

void CarCar::Tracking(int deltaTime) {
	double w3 = 2, w2 = 1;
	double error = 0;
	if (IRsum > 0)
		error = (IRisBlack[0] * (-w3) + IRisBlack[1] * (-w2) + IRisBlack[3] * w2 + IRisBlack[4] * w3) / IRsum;
	else
		error = lastError;
	double dt = deltaTime;
	if (dt <= 0) {
		dt = 50;
		lastError = error;
	}
	integral += error * dt;
	double derivative = (error - lastError) / dt;
	//double correction = Kp * error + Ki * integral + Kd * derivative;
	//int powerCorrection = (int)(correction * forwardspeed);
	int powerCorrection = int(Kp * error + Ki * integral + Kd * derivative);
	int vR = forwardspeed - powerCorrection;  // ex. Tp = 150, 也與w2 & w3有關
	int vL = forwardspeed + powerCorrection;
	if (vR >= 255) vR = 255;
	if (vL >= 255) vL = 255;
	if (vR <= -255) vR = -255;
	if (vL <= -255) vL = -255;
	if (IRsum >= 4) { //與讀到RFID同步
		isInnode = 1;
		turning = 1;
		turntime = 0;
		integral = 0;
		lastError = 0;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		String btMsg = "Track:" + String(motion_duration) + "\n";
		btMsg += "runTime:" + String(curTime - start_time) + "\n";
		dir = next_dir;
		next_dir = WAIT_FOR_COMMAND;
		btMsg += "inn\ndir:" + getDirString(dir);
		Serial3.println(btMsg);

		trackingData.update(motion_duration);

		motion_startTime = curTime;
	} else {
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

void CarCar::stop(int deltaTime) {
	isRunning = 0;
	target_motor_vL = 0;
	target_motor_vR = 0;
	MotorWriting(deltaTime);
}

void CarCar::restart() {
	isInnode = 1;
	turntime = 0;
	//modeState = 0;
	turning = 1;
	dir = FORWARD;
	isRunning = 1;
	lastError = 0;
	integral = 0;

	start_time = millis();
	motion_startTime = start_time;
	trackingData.reset();
	for (int i = 0; i < direction_num; i++) turningData[i].reset();
}

void CarCar::adjust_motor_error(int deltatime) {  //根據目前的速度與差值走直線，不做tracking
	if (!isRunning) {
		target_motor_vL = 0;
		target_motor_vR = 0;
	} else {
		target_motor_vL = forwardspeed;
		target_motor_vR = forwardspeed;
		if (IRsum >= 4) {
			int current_adjust_time = millis() - adjust_start_time;
			if (!adjust_start) {
				adjust_start_time = millis();
				adjust_start = 1;
			} else if (current_adjust_time >= 1500) {
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
	testInnerSpeed = outer * ratio;  // Calculate inner speed safely
	testState = TEST_TRACK;          // Enter tracking state
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
			int deadzoneTime = 70000 / testOuterSpeed;  // Blind turning time to escape the node (adjust if needed)

			// 2. Exit condition: Passed deadzone AND center IR sees the line
			if (currentTurnTime > deadzoneTime && IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3])) {
				testState = TEST_WAIT;
				isRunning = 0;  // Emergency stop!
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

void CarCar::reportData() {
	// Use CSV format for easy computer parsing
	String header = "--- FINAL REPORT ---\nAction,Count,TotalTime(ms),MaxTime(ms),MinTime(ms),AvgTime(ms)";
	Serial.println(header);
	Serial3.println(header);

	// Output Tracking data
	printMotionData("Track", trackingData);

	// Output Turning data for all executed directions
	for (int i = 0; i < direction_num; i++) {
		if (turningData[i].count > 0) {
			String dirName = "Turn_" + getDirString((Direction)i);
			printMotionData(dirName, turningData[i]);
		}
	}

	Serial.println("--- END REPORT ---");
	Serial3.println("--- END REPORT ---");
}

void CarCar::printMotionData(String actionName, MotionData data) {
	unsigned long avgTime = 0;
	unsigned long safeMinTime = 0;

	// Prevent division by zero and raw MAX_ULONG output when count is 0
	if (data.count > 0) {
		avgTime = data.totalTime / data.count;
		safeMinTime = data.minTime;
	}

	// Output to USB Serial
	Serial.print(actionName);
	Serial.print(",");
	Serial.print(data.count);
	Serial.print(",");
	Serial.print(data.totalTime);
	Serial.print(",");
	Serial.print(data.maxTime);
	Serial.print(",");
	Serial.print(safeMinTime);
	Serial.print(",");
	Serial.println(avgTime);

	// Output to Bluetooth Serial3
	Serial3.print(actionName);
	Serial3.print(",");
	Serial3.print(data.count);
	Serial3.print(",");
	Serial3.print(data.totalTime);
	Serial3.print(",");
	Serial3.print(data.maxTime);
	Serial3.print(",");
	Serial3.print(safeMinTime);
	Serial3.print(",");
	Serial3.println(avgTime);
}
