#include "MPU6050_util.h"

CyclicList::CyclicList(int buff_size) {
  this->list = new int16_t[buff_size];
  this->insertion_point = 0;
  this->list_size = buff_size;
}

CyclicList::~CyclicList() {
  delete[] list;
}

int CyclicList::nextIdx(int idx) {
  return ((idx+1)%this->list_size);
}

void CyclicList::append(int value) {
  this->insertion_point = this->nextIdx(this->insertion_point);
  this->list[this->insertion_point] = value;
}

float CyclicList::average() {
  int16_t average = 0;
  for(int i=0; i<this->list_size; i++) {
    average += this->list[i];
    delay(0);
  }
  return ((float)average/(float)list_size);
}

int CyclicList::biggestDifference() {
  int minimum = this->list[0];
  int maximum = this->list[0];
  int curr_value;
  for (int i=0; i<this->list_size; i++) {
    curr_value = this->list[i];
    if (curr_value < minimum) { minimum = curr_value; }
    if (curr_value > maximum) { maximum = curr_value; }
  }
  return (maximum - minimum);
}
