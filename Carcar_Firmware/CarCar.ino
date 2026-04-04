
CarCar::CarCar() {
}

CarCar::~CarCar() {
}

void CarCar::begin() {
  initMotor();
  initIR();
  initRFID();
  isInnode = 0;
  turntime = 0;
  modeState = 0;
  dir = mode[modeState];
  turning = 0;
  isRunning = 0;
}

void CarCar::reading() {
  readRFID();
  readIR();
}
