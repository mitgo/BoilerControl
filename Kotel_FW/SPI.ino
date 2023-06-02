//#include <SPI.h>                    //SPI
#define HC_CS       16         //чипселект 74hc959
bool spiInit() {
  pinMode(HC_CS, OUTPUT);
  hc74 = 0;
  bitSet(hc74,LCD_LED);
  writeHC74(hc74);
  return true;
}

void writeHC74(byte b) {
  digitalWrite(HC_CS, LOW);
  SPI.transfer(b);
  digitalWrite(HC_CS, HIGH);
}
