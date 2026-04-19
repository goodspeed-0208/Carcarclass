
void CarCar::initMotor() {
  pinMode(MotorL_PWML, OUTPUT);
  pinMode(MotorL_I1, OUTPUT);
  pinMode(MotorL_I2, OUTPUT);
  pinMode(MotorR_PWMR, OUTPUT);
  pinMode(MotorR_I3, OUTPUT);
  pinMode(MotorR_I4, OUTPUT);
}

void CarCar::MotorWriting(int deltaTime) {


  int vL = target_motor_vL;
  int vR = target_motor_vR;


  int maxSpeedChange = (int)(maxAcceleration * deltaTime);
  if (isRunning) {
    if (vL > last_motor_vL) vL = min(vL, last_motor_vL + maxSpeedChange);
    else if (vL < last_motor_vL) vL = max(vL, last_motor_vL - maxSpeedChange);
    
    if (vR > last_motor_vR) vR = min(vR, last_motor_vR + maxSpeedChange);
    else if (vR < last_motor_vR) vR = max(vR, last_motor_vR - maxSpeedChange);
  }

  if(vL != last_motor_vL || vR != last_motor_vR){
    last_motor_vL = vL;
    last_motor_vR = vR;

    double vL_error = abs(vL)*0.0302-0.202;
    double vR_error = abs(vR)*0.0302-0.202;

    if (vL > 0) vL = max(0, vL + (int)(vL_error+0.5)) ; //四捨五入
    else if (vL < 0) vL = min(0, vL - (int)(vL_error+0.5));
    
    if (vR > 0) vR = max(0, vR + (int)(vR_error+0.5)) ; 
    else if (vR < 0) vR = min(0, vR - (int)(vR_error+0.5));


    /*if (vL > 0) vL += motor_error;
    else if (vL < 0) vL -= motor_error;
    if (vR > 0) vR -= motor_error;
    else if (vR < 0) vR += motor_error;*/
    
    if (vL >= 255) vL = 255;
    if (vL <= -255) vL = -255;
    if (vR >= 255) vR = 255;
    if (vR <= -255) vR = -255;

    if (vR >= 0) {
      digitalWrite(MotorR_I3, LOW);
      digitalWrite(MotorR_I4, HIGH);
      //這邊的Motor第幾個對應到的High/Low是助教的車對應到的，請自己測試自己車該怎麼填！
    } else if (vR < 0) {
      digitalWrite(MotorR_I3, HIGH);
      digitalWrite(MotorR_I4, LOW);
      vR = -vR;  //因為analogWrite只吃正數，所以如果本來是負數，就要乘-1
    }
    if (vL >= 0) {
      digitalWrite(MotorL_I1, LOW);
      digitalWrite(MotorL_I2, HIGH);
      //這邊的Motor第幾個對應到的High/Low是助教的車對應到的，請自己測試自己車該怎麼填！
    } else if (vL < 0) {
      digitalWrite(MotorL_I1, HIGH);
      digitalWrite(MotorL_I2, LOW);
      vL = -vL;  //因為analogWrite只吃正數，所以如果本來是負數，就要乘-1
    }
    analogWrite(MotorL_PWML, vL);
    analogWrite(MotorR_PWMR, vR);

  }
}
