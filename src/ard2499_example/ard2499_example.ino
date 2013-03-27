#include <Wire.h>
#include <Ard2499.h>

Ard2499 ard2499board1;

byte confChan=0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  Wire.begin();
  ard2499board1.begin(ARD2499_ADC_ADDR_ZZZ, ARD2499_EEP_ADDR_ZZ);
  ard2499board1.ltc2499ChangeConfiguration(LTC2499_CONFIG2_60_50HZ_REJ);
  confChan = ard2499board1.ltc2499ChangeChannel(LTC2499_CHAN_TEMPERATURE);
}
byte i=0;

void loop() {
  // print the results to the serial monitor:

  byte retval = 0;
 
  Serial.print("eeprom mac = [");
  Serial.print(ard2499board1.eui48Get());
  Serial.print("]\n");
  
  Serial.print(" write eeprom[0] ");
  Serial.print(i);
  retval = ard2499board1.eepromWrite(0, i, true);
  Serial.print(" retval=");
  Serial.print(retval);
  Serial.print("\n");
  i++;

  Serial.print("read eeprom[0] ");
  Serial.print(ard2499board1.eepromRead(0, true));
  Serial.print("\n");

  ard2499board1.ltc2499ChangeChannel(LTC2499_CHAN_SINGLE_0P);
  Serial.print("Channel 0 SE");
  Serial.print(" = [");
  Serial.print(ard2499board1.ltc2499ReadAndChangeChannel(LTC2499_CHAN_TEMPERATURE));
  Serial.print("]\n");

  Serial.print("Temp = [");
  Serial.print(ard2499board1.ltc2499ReadTemperature(ARD2499_TEMP_F));
  Serial.print(" F]\n");
  
  
  delay(1000);
}
