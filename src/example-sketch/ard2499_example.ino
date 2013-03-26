#include <Wire.h>
#include <ard2499.h>

Ard2499 ard2499;

#define ARD2499_ADC_ADDRESS 0x45
#define ARD2499_EEP_ADDRESS ARD2499_EEP_ADDR_ZZ
byte confRet = 0xFF;
byte confChan = 0xFF;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  Wire.begin();
  confRet = ard2499.begin(ARD2499_ADC_ADDRESS, ARD2499_EEP_ADDRESS);
  confRet = ard2499.ltc2499ChangeConfiguration(LTC2499_CONFIG2_60_50HZ_REJ);
  confChan = ard2499.ltc2499ChangeChannel(LTC2499_CHAN_TEMPERATURE);
}
byte i=0;

void loop() {
  // print the results to the serial monitor:

  byte retval = 0;
 
  unsigned long tempK = ard2499.ltc2499ReadRaw();

  
  Serial.print("eeprom mac = [");
  Serial.print(ard2499.eui48Get());
  Serial.print("]\n");
  
  Serial.print("read[0] ");
  Serial.print(ard2499.eepromRead(0, true));
  Serial.print("\n");

  ard2499.ltc2499ChangeChannel(LTC2499_CHAN_SINGLE_0P);
  Serial.print("Channel 0 SE");
  Serial.print(" = [");
  Serial.print(ard2499.ltc2499ReadAndChangeChannel(LTC2499_CHAN_TEMPERATURE));
  Serial.print("]\n");

  Serial.print("Temp = [");
  Serial.print(ard2499.ltc2499ReadTemperatureDeciK());
  Serial.print(" dK]\n");
  
/*  Serial.print(" write[0] ");
  Serial.print(i);
  retval = ard2499.eepromWrite(0, i, true);
  Serial.print(" retval=");
  Serial.print(retval);*/
  Serial.print("\n");
  i++;
  
  delay(1000);
}
