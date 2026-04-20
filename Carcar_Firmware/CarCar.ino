
CarCar::CarCar() {
}

CarCar::~CarCar() {
}

void CarCar::begin() {
  initMotor();
  initIR();
  initRFID();

  dir = FORWARD;

  start_time = millis();
  motion_startTime = start_time;
}

void CarCar::reading() {
  readRFID();
  readIR();
}
