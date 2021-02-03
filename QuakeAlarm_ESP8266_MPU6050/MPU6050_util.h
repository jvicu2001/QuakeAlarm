#ifndef H_MPU6050_UTIL
#define H_MPU6050_UTIL

#include <Arduino.h>
#include <Wire.h>

// Memory map from https://invensense.tdk.com/wp-content/uploads/2015/02/MPU-6000-Register-Map1.pdf

#define ACCEL_XOUT 0x3B
#define ACCEL_YOUT 0x3D
#define ACCEL_ZOUT 0x3F
#define TEMP_OUT 0x41
#define GYRO_XOUT 0x43
#define GYRO_YOUT 0x45
#define GYRO_ZOUT 0x47

#define CONFIG 0x1A
#define GYRO_CONFIG 0x1B
#define ACCEL_CONFIG 0x1C

#define ALL_AXIS true  // If true, all axis get sumed. If false, only the horizontal axis get sumed.

class CyclicList {
  public:
    CyclicList(int buff_size);
    ~CyclicList();
    void append(int value);
    int nextIdx(int idx);
    float average();
    int biggestDifference();
  private:
    int16_t * list;
    int insertion_point;
    int list_size;
};

class MPU6050 {
  public:
    void readData();
    void activate(unsigned int filter_level);
    MPU6050(bool I2C_Alt_Addr);
    ~MPU6050();
    int getVAxis();
    int16_t getRawAcX();
    int16_t getRawAcY();
    int16_t getRawAcZ();
    /*int16_t getRawTmp();
    int16_t getRawGyX();
    int16_t getRawGyY();
    int16_t getRawGyZ();
    int16_t getMinAcc();
    int16_t getMaxAcc();*/
    long getCurrentAcc();
    long getAccDiff();
    /*int16_t getMinDiff();
    int16_t getMaxDiff();*/
    long getAxisSum();
    long getBiggestDiff();
    long getImmediateDiff();
    float getDiffAverage();
    
  private:
    void setDefaultParameters();
    //  int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ,minAcc,maxAcc,minDiff,maxDiff,lastAcc,currAcc;
    int16_t AcX,AcY,AcZ;
    long lastAcc,currAcc;
    
    int VAxis;
    int I2C_Addr;
    CyclicList * diff_buffer;
    CyclicList * value_buffer;
};

#endif
