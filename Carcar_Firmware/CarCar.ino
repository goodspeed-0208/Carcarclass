
CarCar::CarCar(){
  
}

CarCar::~CarCar(){

}

void CarCar::begin() {
  initMotor();
  initIR();
  initRFID();
  isInnode = 0;
  turntime = 0;
  dir = 3;
  turning = 0;
  isRunning = 0;

  modeState = 0; 
}

void CarCar::reading(){
  readRFID();
  readIR();
}

