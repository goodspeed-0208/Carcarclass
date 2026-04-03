
void CarCar::initMotor() {
  pinMode(MotorL_PWML, OUTPUT);
  pinMode(MotorL_I1, OUTPUT);
  pinMode(MotorL_I2, OUTPUT);
  pinMode(MotorR_PWMR, OUTPUT);
  pinMode(MotorR_I3, OUTPUT);
  pinMode(MotorR_I4, OUTPUT);
}

void CarCar::MotorWriting(double vL, double vR) {
  if (vL > 0) vL += 5;
  else if (vL < 0) vL -= 5;
  if (vL > 255) vL = 255;
  if (vL < -255) vL = -255;

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