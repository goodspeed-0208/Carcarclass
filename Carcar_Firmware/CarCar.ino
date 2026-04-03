
CarCar::CarCar(){
  
}

CarCar::~CarCar(){

}

void CarCar::begin() {
  initMotor();
  initIR();
  initRFID();
  lastIRsum = 0;
  isInode = 0;
  turntime = 0;
  dir = 3;
  turning = 0;
  running = 0;

  modeState = 0; 
}

