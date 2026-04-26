
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
			}
			else if (dir == LEFT) turnleft();
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

		/*if (!turning && millis() - motion_startTime >= 180) {  //紀錄直行平均速度
			trackCount++;
			sum_vL += last_motor_vL;
			sum_vR += last_motor_vR;
			averagevL = sum_vL / trackCount;
			averagevR = sum_vR / trackCount;
		} else if (turning && dir != BACKWARD) {
			sum_vL = 0;
			sum_vR = 0;
			trackCount = 0;
		}*/
	}
}

void CarCar::goForward() {
	target_motor_vL = forwardspeed, target_motor_vR = forwardspeed;
	if (extremeModeOn) target_motor_vL = extremeSpeed, target_motor_vR = extremeSpeed;

	if (turntime >= Min_forward_turntime && IRsum <= 2 && IRsum >= 1) {
		turning = 0;
		isInnode = 0;
		lastActionWasTurn = false;
		lastActionWasUTurn = false;
		//modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		char btBuffer[64];
		snprintf(btBuffer, sizeof(btBuffer), "%s:%lu,OUTN",
		         getDirString(dir).c_str(), motion_duration);
		Serial3.println(btBuffer);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
	}
}

void CarCar::turnleft() {
	target_motor_vL = turnInnerSpeed, target_motor_vR = turnOuterSpeed;
	if (turntime >= Min_rightleft_turntime && (IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3]))) {
		turning = 0;
		isInnode = 0;
		lastActionWasTurn = true;
		lastActionWasUTurn = false;
		//modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		char btBuffer[64];
		snprintf(btBuffer, sizeof(btBuffer), "%s:%lu,OUTN",
		         getDirString(dir).c_str(), motion_duration);
		Serial3.println(btBuffer);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
	}
}

void CarCar::turnright() {
	target_motor_vL = turnOuterSpeed, target_motor_vR = turnInnerSpeed;
	if (turntime >= Min_rightleft_turntime && (IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3]))) {
		turning = 0;
		isInnode = 0;
		lastActionWasTurn = true;
		lastActionWasUTurn = false;
		//modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		char btBuffer[64];
		snprintf(btBuffer, sizeof(btBuffer), "%s:%lu,OUTN",
		         getDirString(dir).c_str(), motion_duration);
		Serial3.println(btBuffer);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
	}
}

void CarCar::turnback() {
	target_motor_vL = turnBackSpeed_first, target_motor_vR = -turnBackSpeed_first;
	if(turntime >= Min_turnback_turntime){
		target_motor_vL = turnBackSpeed_second;
		target_motor_vR = -turnBackSpeed_second;
	}
	if (turntime >= Min_turnback_turntime && (!IRisBlack[0] && (IRisBlack[1] || IRisBlack[2] || IRisBlack[3] || IRisBlack[4]))) {
		turning = 0;
		isInnode = 0;
		lastActionWasTurn = true;
		lastActionWasUTurn = true;
		//modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		char btBuffer[64];
		snprintf(btBuffer, sizeof(btBuffer), "%s:%lu,OUTN",
		         getDirString(dir).c_str(), motion_duration);
		Serial3.println(btBuffer);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
	}
}

void CarCar::goBackward() {  //turning持續到回到上一個節點，目前功能尚不齊全
	target_motor_vL = -backwardspeed_first, target_motor_vR = -backwardspeed_first;
	if (isInnode && IRsum <= 2) {
		isInnode = 0;
		String btMsg = "OUTN";
		Serial3.println(btMsg);
	}
	if (turntime >= Min_backward_turntime){
		target_motor_vL = -backwardspeed_second, target_motor_vR = -backwardspeed_second;
	}
	if (turntime >= Min_backward_turntime && !isInnode && IRsum >= 4) {
		turning = 1;
		isInnode = 1;
		lastActionWasTurn = true;
		lastActionWasUTurn = false;
		//modeState = (modeState + 1) % 8;
		//sum_vL = 0;
		//sum_vR = 0;
		//trackCount = 0;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;

		char btBuffer[64];
		snprintf(btBuffer, sizeof(btBuffer), "%s:%lu || INN,dir:%s",
		         getDirString(dir).c_str(), motion_duration, getDirString(next_dir).c_str());
		Serial3.println(btBuffer);

		turningData[dir].update(motion_duration);

		dir = next_dir;

		motion_startTime = curTime;
	}
}

void CarCar::turnleft_after_backward() {
	target_motor_vL = turnOuterSpeed_back, target_motor_vR = turnInnerSpeed_back;
	if (turntime >= Min_rightleft_turntime &&(IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3]))) {
		turning = 0;
		isInnode = 0;
		lastActionWasTurn = true;
		lastActionWasUTurn = false;
		//modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		char btBuffer[64];
		snprintf(btBuffer, sizeof(btBuffer), "%s:%lu,OUTN",
		         getDirString(dir).c_str(), motion_duration);
		Serial3.println(btBuffer);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
	}
}

void CarCar::turnright_after_backward() {
	target_motor_vL = turnInnerSpeed_back, target_motor_vR = turnOuterSpeed_back;
	if (turntime >= Min_rightleft_turntime && (IRisBlack[0] == 0 && IRisBlack[4] == 0 && (IRisBlack[2] || IRisBlack[1] || IRisBlack[3]))) {
		turning = 0;
		isInnode = 0;
		lastActionWasTurn = true;
		lastActionWasUTurn = false;
		//modeState = (modeState + 1) % 8;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		char btBuffer[64];
		snprintf(btBuffer, sizeof(btBuffer), "%s:%lu,OUTN",
		         getDirString(dir).c_str(), motion_duration);
		Serial3.println(btBuffer);
		turningData[dir].update(motion_duration);
		motion_startTime = curTime;
	}
}

void CarCar::wait_for_command() {
	target_motor_vL = 0, target_motor_vR = 0;

	if (next_dir != WAIT_FOR_COMMAND) {
		dir = next_dir;
		next_dir = WAIT_FOR_COMMAND;
		lastActionWasTurn = true;
		lastActionWasUTurn = false;
		turntime = 0;
		motion_startTime = millis();
	}
}

void CarCar::Tracking(int deltaTime) {
	/*if (IRsum == 0) {  //出軌情況
		target_motor_vL = 0;
		target_motor_vR = 0;

		/*last_motor_vL = 0;
		last_motor_vR = 0;

		integral = 0;
		lastError = 0;

		return;
	}*/

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
	//integral += error * dt;
	double derivative = (error - lastError) / dt;
	//double correction = Kp * error + Ki * integral + Kd * derivative;
	//int powerCorrection = (int)(correction * forwardspeed);
	//int powerCorrection = int(Kp * error + Ki * integral + Kd * derivative); //移到底下

	int currentBaseSpeed = forwardspeed;

	currentSegmentType = 0;

	if (extremeModeOn) {
		// 如果 next_dir 已經是 FORWARD，代表未來要直走
		bool willGoStraight = (next_dir == FORWARD);
		bool willReadCard = (next_dir == BACKWARD || next_dir == TURN_BACK || next_dir == STAY_STOP);

		if (willReadCard) currentSegmentType = 4;
		else if (lastActionWasUTurn) currentSegmentType = 5;                     //迴轉 -> 直走 (距離短)
		else if (lastActionWasTurn && !willGoStraight) currentSegmentType = 0;   // 轉彎 -> 轉彎 (一般150)
		else if (!lastActionWasTurn && willGoStraight) currentSegmentType = 1;   // 直走 -> 直走 (極限200)
		else if (lastActionWasTurn && willGoStraight) currentSegmentType = 2;    // 轉彎 -> 直走 (加速段)
		else if (!lastActionWasTurn && !willGoStraight) currentSegmentType = 3;  // 直走 -> 轉彎 (減速段)

		unsigned long timeSinceLastNode = millis() - motion_startTime;

		if (currentSegmentType == 1) {
			currentBaseSpeed = extremeSpeed;
		} else if (currentSegmentType == 2) {  // 加速段
			if (timeSinceLastNode >= extremeAccelDelay) currentBaseSpeed = extremeSpeed;
		} else if (currentSegmentType == 3) {  // 減速段
			if (timeSinceLastNode < extremeDecelDelay) currentBaseSpeed = extremeSpeed;
		} else if (currentSegmentType == 4) {
			unsigned long curTime = millis();
			unsigned long motion_duration = curTime - motion_startTime;
			if(motion_duration >= inUturnDecelTime){
				currentBaseSpeed = inUturnSpeed_decel;
			}
		} else if (currentSegmentType == 5) {
			// 【迴轉恢復段的速度策略】
			// 因為迴轉後車身最不穩，這裡我們預設「不加速」，整段用 150 穩穩走完。
			// 所以我們什麼都不用寫，它會自然保持在預設的 currentBaseSpeed = 150
		}
	}

	double activeKp = Kp, activeKi = Ki, activeKd = Kd;
	if (extremeModeOn && currentBaseSpeed == extremeSpeed) {
		activeKp = exKp;
		activeKi = exKi;
		activeKd = exKd;
	}

	int powerCorrection = int(activeKp * error + activeKi * integral + activeKd * derivative);
	int vR = currentBaseSpeed - powerCorrection;  // ex. Tp = 150, 也與w2 & w3有關
	int vL = currentBaseSpeed + powerCorrection;
	if (vR >= 255) vR = 255;
	if (vL >= 255) vL = 255;
	if (vR <= -255) vR = -255;
	if (vL <= -255) vL = -255;

	if(IRsum >= 4 && !isFindingRFID && (next_dir == BACKWARD || next_dir == TURN_BACK || next_dir == STAY_STOP)){
		isFindingRFID = true;
		RFID_startTime = millis();
	}

	if (IRsum >= 4 && ((next_dir != BACKWARD && next_dir != TURN_BACK && next_dir != STAY_STOP) || (isFindingRFID && millis() >= RFID_startTime + Max_RFID_findtime))) {  //與讀到RFID同步
		isFindingRFID = false;
		isInnode = 1;
		turning = 1;
		turntime = 0;
		integral = 0;
		lastError = 0;

		unsigned long curTime = millis();
		unsigned long motion_duration = curTime - motion_startTime;
		dir = next_dir;
		next_dir = WAIT_FOR_COMMAND;

		char btBuffer[64];
		snprintf(btBuffer, sizeof(btBuffer), "Tr:%lu || INN,dir:%s",
		          motion_duration, getDirString(dir).c_str());
		Serial3.println(btBuffer);

		trackingData[currentSegmentType].update(motion_duration);
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
	turntime = Min_forward_turntime;
	dir = FORWARD;
	isRunning = 1;
	lastError = 0;
	integral = 0;

	// 新增：重設 RFID 紀錄
  visitedCount = 0;
  for (int i = 0; i < MAX_VISITED; i++) {
    visitedUIDs[i] = "";
  }

	start_time = millis();
	motion_startTime = start_time;
	for (int i = 0; i < 6; i++) trackingData[i].reset();
	for (int i = 0; i < direction_num; i++) turningData[i].reset();
}

/*void CarCar::adjust_motor_error(int deltatime) {  //根據目前的速度與差值走直線，不做tracking
	if (!isRunning) {
		target_motor_vL = 0;
		target_motor_vR = 0;
	} else {
		target_motor_vL = -forwardspeed;
		target_motor_vR = -forwardspeed;
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
}*/

/*void CarCar::start_turn_test(char dir, int outer, float ratio) {
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
}*/

void CarCar::reportData() {
	// Use CSV format for easy computer parsing
	Serial3.println(F("--- FINAL REPORT ---"));
	delay(40);
	Serial3.println(F("Action,Count,TotalTime(ms),MaxTime(ms),MinTime(ms),AvgTime(ms)"));

	delay(50);

	// Output Tracking data
	if (trackingData[0].count > 0) printMotionData("Track_150", trackingData[0]);
	if (trackingData[1].count > 0) printMotionData("Track_200", trackingData[1]);
	if (trackingData[2].count > 0) printMotionData("Track_Accel", trackingData[2]);
	if (trackingData[3].count > 0) printMotionData("Track_Decel", trackingData[3]);
	if (trackingData[4].count > 0) printMotionData("Track_UTurn_in", trackingData[4]);
	if (trackingData[5].count > 0) printMotionData("Track_UTurn_out", trackingData[5]);

	// Output Turning data for all executed directions
	for (int i = 0; i < direction_num; i++) {
		if (turningData[i].count > 0) {
			String dirName = "Turn_" + getDirString((Direction)i);
			printMotionData(dirName, turningData[i]);
		}
	}

	Serial3.println(F("--- END REPORT ---"));

	Serial3.println("total time: " + String(millis() - start_time));

	delay(20);
}

void CarCar::printMotionData(String actionName, MotionData data) {
	unsigned long avgTime = 0;
	unsigned long safeMinTime = 0;

	// Prevent division by zero and raw MAX_ULONG output when count is 0
	if (data.count > 0) {
		avgTime = data.totalTime / data.count;
		safeMinTime = data.minTime;
	}

	// Output to Bluetooth Serial3
	String btmsg = actionName + "," + data.count + "," + data.totalTime + "," + data.maxTime + "," + safeMinTime + "," + avgTime;
	Serial3.println(btmsg);

	delay(40);
}
