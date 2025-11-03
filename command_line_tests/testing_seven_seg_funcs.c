#include <stdio.h>
#include <stdint.h>

void setSevenSeg(int decimalValue) {
  uint8_t firstDigit = decimalValue/10; //integer division, should only accept the tens place
  uint8_t secondDigit = decimalValue % 10; //this should give the ones place

  //printf("first digit is: %d\n", firstDigit);
  //printf("second digit is: %d\n", secondDigit);

}

int main() {
  printf("hello from Jungleland \n");

  printf("------------------------");

  setSevenSeg(48);  
  setSevenSeg(0);  
  setSevenSeg(11);  
  setSevenSeg(28);  
  setSevenSeg(30);
}


