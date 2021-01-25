#include "MPU6050_util.h"

//  Basic getter functions
int16_t MPU6050::getRawAcX() {
  return this->AcX;
}
int16_t MPU6050::getRawAcY() {
  return this->AcY;
}
int16_t MPU6050::getRawAcZ() {
  return this->AcZ;
}
/*int16_t MPU6050::getRawTmp() { return this->Tmp; }
  int16_t MPU6050::getRawGyX() { return this->GyX; }
  int16_t MPU6050::getRawGyY() { return this->GyY; }
  int16_t MPU6050::getRawGyZ() { return this->GyZ; }*/
int16_t MPU6050::getCurrentAcc() {
  return this->currAcc;
}
/*int16_t MPU6050::getMinAcc() { return(this->minAcc); }
  int16_t MPU6050::getMaxAcc() { return(this->maxAcc); }*/
int16_t MPU6050::getAccDiff() {
  return (this->currAcc - this->lastAcc);
}
/*int16_t MPU6050::getMinDiff() { return(this->minDiff); }
  int16_t MPU6050::getMaxDiff() { return(this->maxDiff); }*/
float MPU6050::getDiffAverage() {
  return this->diff_buffer->average();
}

//  Read Accelerometer, Gyroscope and Temperature data from the MPU6050's registers
void MPU6050::readData() {
  Wire.beginTransmission(this->I2C_Addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(this->I2C_Addr, 6, true); // request a total of ~14~ 6 registers
  this->AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)
  this->AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  this->AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  /*this->Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    this->GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    this->GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    this->GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)*/


  //  Writing buffered and processed data to memory
  this->lastAcc = this->currAcc;
  this->currAcc = this->getAxisSum();
  this->diff_buffer->append(abs(this->currAcc - this->lastAcc));
}

//  Setting up procesed data, for analysis, most aren't needed
void MPU6050::setDefaultParameters() {
  delay(50);    // Let initial readings normalize themselves before taking data

  //  Figuring out the Vertical Axis
  while ((this->AcX == this->AcY) and (this->AcX == this->AcZ)) {
    this->readData();
    delay(0);
    if ((this->AcX != this->AcY) and (this->AcX != this->AcZ)) {
      if (abs(this->AcX) > abs(this->AcY)) {
        if (abs(this->AcX) > abs(this->AcZ)) {
          this->VAxis = 0;
        }
        else {
          this->VAxis = 2;
        }
      }
      else {
        if (abs(this->AcY) > abs(this->AcZ)) {
          this->VAxis = 1;
        }
        else {
          this->VAxis = 2;
        }
      }
    }
  }
  /*
    //  Getting normal data bounds, can be useful for the NodeMCU to determine thresholds by itself.
    int16_t axisSum = this->getAxisSum();
    this->minAcc = axisSum;
    this->maxAcc = axisSum;
    this->minDiff = 0;
    this->maxDiff = 0;

    unsigned long start_time = millis();

    // Reading again to have at least two values
    this->readData();

    //  Getting top and bottom bounds
    while (millis() < (start_time + 500)){
    this->readData();
    axisSum = this->getAxisSum();
    int diff = this->getAccDiff();
    if (axisSum < this->minAcc) { this->minAcc = axisSum; }
    if (axisSum > this->maxAcc) { this->maxAcc = axisSum; }
    if (diff < this->minDiff) { this->minDiff = diff; }
    if (diff > this->maxDiff) { this->maxDiff = diff; }
    delay(20);
    }
  */
}

//  Adding up the two horizontal planes, simplifying processing data.
int16_t MPU6050::getAxisSum() {
  switch (this->VAxis) {
    case 0:
      return (this->AcY + this->AcZ);
      break;
    case 1:
      return (this->AcX + this->AcZ);
      break;
    case 2:
      return (this->AcX + this->AcY);
      break;
  }
}

//  Wake up the MPU6050, or Power cycle it if it was already running (resetting the NodeMCU wihle operating causes this)
void MPU6050::activate(unsigned int filter_level) {
  Wire.beginTransmission(this->I2C_Addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.endTransmission(false);
  Wire.requestFrom(this->I2C_Addr, 1, true);
  int power_status = (Wire.read() & 0x80) >> 7;

  if (power_status == 0) {
    Wire.beginTransmission(this->I2C_Addr);
    Wire.write(0x6B);           // PWR_MGMT_1 register
    Wire.write(0x80);           // Sets DEVICE_RESET to 1, resetting all Registers
    Wire.endTransmission(true);

    delay(500);                 // Waiting for the device to reset
  }

  //  Asserting that the device will be awake
  Wire.beginTransmission(this->I2C_Addr);
  Wire.write(0x6B);             // PWR_MGMT_1 register
  Wire.write(0x00);             // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  delay(500);

  //  Output filtering setup
  Wire.beginTransmission(this->I2C_Addr);
  Wire.write(0x1A);             // Selects CONFIG Register
  if (filter_level < 7) {
    Wire.write(filter_level);   // Sets DLPF_CFG. Filtering values vary from 0 to 6, 6 being the strongest.
  }
  else {
    Wire.write(0x06);           // Sets maximum filtering for the output if an invalid value is given.
  }
  Wire.endTransmission(true);


  //  Sensivity setup
  Wire.beginTransmission(this->I2C_Addr);
  Wire.write(ACCEL_CONFIG);
  Wire.write(0x00);                       /*  Select ACCEL_CONFIG
                                              Disables Accel self-test and
                                              sets Full Scale Range to +-2g
*/
  Wire.endTransmission(true);

  //Wire.beginTransmission(this->I2C_Addr);
  //Wire.write(GYRO_CONFIG);
  //Wire.write(0x00);
  /*  Select GYRO_CONFIG
      Disables Accel self-test and
      sets Full Scale Range to +-250°/s
  */
  //Wire.endTransmission(true);

  this->setDefaultParameters(); // Sets up additional parameters.


  // Filling up the cyclic list to avoid erroneous difference values
  for(int i=0; i<20; i++) {
    this->readData();
  }
}

// MPU6050 Contructor.
MPU6050::MPU6050(bool I2C_Alt_Addr) {
  // Sets the I2C Address depending if AD0 is connected to VCC or GND
  if (I2C_Alt_Addr) {
    this->I2C_Addr = 0x69;
  }
  else {
    this->I2C_Addr = 0x68;
  }

  this->diff_buffer = new CyclicList(20); // Creates CyclicList used for value difference buffer
}

MPU6050::~MPU6050() {
  delete this->diff_buffer;
}
