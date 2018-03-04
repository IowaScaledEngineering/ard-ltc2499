/*************************************************************************
Title:    ARD-LTC2499 Auto Board Test
Authors:  Michael Petersen <railfan@drgw.net>
File:     $Id: $
License:  GNU General Public License v3

LICENSE:
    Copyright (C) 2017 Nathan D. Holmes & Michael D. Petersen

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

*************************************************************************/

#include <Wire.h>
#include <Ard2499.h>

#define VREF 4.096

#define GREEN_LED  2
#define RED_LED    3

#define REG_PIN    2
#define VREF_PIN   3

Ard2499 ard2499board1;

byte confChan=0;

void setup()
{
	pinMode(GREEN_LED, OUTPUT);
	pinMode(RED_LED, OUTPUT);
	
	// initialize serial communications at 9600 bps:
	Serial.begin(115200);
	while(!Serial);
	Wire.begin();
	
	// Configure Arduino ADC
	analogReference(EXTERNAL);
	
	ard2499board1.begin(ARD2499_ADC_ADDR_ZZZ, ARD2499_EEP_ADDR_ZZ, VREF * 1000.0);
	ard2499board1.ltc2499ChangeConfiguration(LTC2499_CONFIG2_60_50HZ_REJ);
	ard2499board1.ltc2499ChangeChannel(LTC2499_CHAN_SINGLE_0P);
}

byte i=0;
byte fail = 0;

void loop()
{
	bool result = false;
	int adc = 0;
	float voltage = 0.0;

	float va = VREF / 2.0;
	float vb = VREF / 4.0;
	float deltaV = va - vb;

	fail = 0;
	
	// FIXME: Redefine deltaV due to board layout error (LT5400 pinout wrong)
	deltaV = VREF * ((6.25/7.25)-(5.0/7.25));

	writeAnalogSwitch(0x00);

	Serial.print("VREF = ");
	Serial.print(VREF, 3);
	Serial.print("V\n\n");

	Serial.println("\nBasic Tests");

	// Regulator Test
	// +/-3% limits
	adc = analogRead(REG_PIN);
	voltage = (4.096 * adc / 1024.0) * (3.0 / 2.0);
	Serial.print("RegV: ");
	Serial.print(voltage, 3);
	Serial.print("V ... ");
	result = ((voltage > 4.85) && (voltage < 5.15));
	testResult(result);

	// LT6654 Test (crude pass/fail, not accuracy)
	// +/-1% limits
	adc = analogRead(VREF_PIN);
	voltage = (4.096 * adc / 1024.0) * (5.0 / 4.0);
	Serial.print("Vref: ");
	Serial.print(voltage, 3);
	Serial.print("V ... ");
	if(VREF < 2)
		result = ((voltage > 1.20) && (voltage < 1.30));
	else
		result = ((voltage > 4.05) && (voltage < 4.15));
	testResult(result);

	// EEPROM Address Test
	Serial.println("\nEEPROM Address");

	writeAnalogSwitch(0x00);
	Serial.print("EEPROM 0x53 (open)... ");
	result = findI2CSlave(0x53);
	testResult(result);

	writeAnalogSwitch(0x40);
	Serial.print("EEPROM 0x52  (JP7)... ");
	result = findI2CSlave(0x52);
	testResult(result);

	writeAnalogSwitch(0x80);
	Serial.print("EEPROM 0x51  (JP8)... ");
	result = findI2CSlave(0x51);
	testResult(result);

	// EEPROM Read/Write Test
	Serial.println("\nEEPROM Read/Write");

	writeAnalogSwitch(0x00);
	Serial.print("EEPROM MAC... ");
	Serial.println(ard2499board1.eui48Get());

	Serial.print("Write 0xAA... ");
	ard2499board1.eepromWrite(0, 0xAA, true);
	result = (0xAA == ard2499board1.eepromRead(0, true));
	testResult(result);

	Serial.print("Write 0x55... ");
	ard2499board1.eepromWrite(0, 0x55, true);
	result = (0x55 == ard2499board1.eepromRead(0, true));
	testResult(result);

	// LTC2499 Address Test
	Serial.println("\nADC Address");

	writeAnalogSwitch(0x00);
	Serial.print("EEPROM 0x45    (open)... ");
	result = findI2CSlave(0x45);
	testResult(result);

	writeAnalogSwitch(0x02);
	Serial.print("EEPROM 0x46 (JP4 Top)... ");
	result = findI2CSlave(0x46);
	testResult(result);

	writeAnalogSwitch(0x01);
	Serial.print("EEPROM 0x44 (JP4 Bot)... ");
	result = findI2CSlave(0x44);
	testResult(result);

	writeAnalogSwitch(0x04);
	Serial.print("EEPROM 0x54 (JP5 Top)... ");
	result = findI2CSlave(0x54);
	testResult(result);

	writeAnalogSwitch(0x08);
	Serial.print("EEPROM 0x36 (JP5 Bot)... ");
	result = findI2CSlave(0x36);
	testResult(result);

	writeAnalogSwitch(0x20);
	Serial.print("EEPROM 0x66 (JP6 Top)... ");
	result = findI2CSlave(0x66);
	testResult(result);

	writeAnalogSwitch(0x10);
	Serial.print("EEPROM 0x24 (JP6 Bot)... ");
	result = findI2CSlave(0x24);
	testResult(result);

	// LTC2499 Analog Input Test
	// +/-0.1% limits
	Serial.println("\nADC Analog Input");

	writeAnalogSwitch(0x00);
	float minV = deltaV * 0.999;
	float maxV = deltaV * 1.001;
	Serial.print("Expected Range = ");
	Serial.print(minV, 6);
	Serial.print("V - ");
	Serial.print(maxV, 6);
	Serial.print("V\n");

	Serial.print("ADC CH0-CH1:   ");
	ard2499board1.ltc2499ChangeChannel(LTC2499_CHAN_DIFF_0P_1N);
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_2P_3N);
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH2-CH3:   ");
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_4P_5N);
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH4-CH5:   ");
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_6P_7N);
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH6-CH7:   ");
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_8P_9N);
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH8-CH9:   ");
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_10P_11N);
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH10-CH11: ");
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_12P_13N);
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH12-CH13: ");
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_14P_15N);
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH14-CH15: ");
	voltage = ard2499board1.ltc2499ReadVoltage();
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	
	// Summarize test results
	Serial.print("\n\n");
	if(fail)
	{
		Serial.println("********************");
		Serial.println(" FAILED...");
		Serial.println("********************");
	}
	else
	{
		Serial.println("********************");
		Serial.println(" PASSED!");
		Serial.println("   INSTALL JUMPERS!");
		Serial.println("********************");
	}
	Serial.print("\n");

	while(!Serial.available());
	while(Serial.available())
	{
		Serial.read();  // Flush the buffer
	}
}

void testResult(byte result)
{
	if(result)
	{
		Serial.println("Pass!");
	}
	else
	{
		Serial.println("*** Fail ***");
		fail = 1;
	}
}

void writeAnalogSwitch(byte value)
{
	Wire.beginTransmission(0x4B);
	Wire.write(value);
	Wire.endTransmission();
	delay(200);  // Wait so system (2499?) can respond to new address
}

byte findI2CSlave(uint8_t addr)
{
	byte stat, done = 0;
	Wire.beginTransmission(addr);
	stat = Wire.endTransmission();
	return(!stat);
}





/*

skipJumpers:

  for(i=0; i<17; i++)
  {
    byte newChannel;
    byte adjacentChannel;
    switch(i)
    {
       case 0:
          newChannel = LTC2499_CHAN_SINGLE_0P;
          adjacentChannel = LTC2499_CHAN_SINGLE_1P;
          break;
          
       case 1:
          newChannel = LTC2499_CHAN_SINGLE_1P;
          adjacentChannel = LTC2499_CHAN_SINGLE_2P;
          break;

       case 2:
          newChannel = LTC2499_CHAN_SINGLE_2P;
          adjacentChannel = LTC2499_CHAN_SINGLE_3P;
          break;

       case 3:
          newChannel = LTC2499_CHAN_SINGLE_3P;
          adjacentChannel = LTC2499_CHAN_SINGLE_4P;
          break;

       case 4:
          newChannel = LTC2499_CHAN_SINGLE_4P;
          adjacentChannel = LTC2499_CHAN_SINGLE_5P;
          break;
          
       case 5:
          newChannel = LTC2499_CHAN_SINGLE_5P;
          adjacentChannel = LTC2499_CHAN_SINGLE_6P;
          break;

       case 6:
          newChannel = LTC2499_CHAN_SINGLE_6P;
          adjacentChannel = LTC2499_CHAN_SINGLE_7P;
          break;

       case 7:
          newChannel = LTC2499_CHAN_SINGLE_7P;
          adjacentChannel = LTC2499_CHAN_SINGLE_8P;
          break;

       case 8:
          newChannel = LTC2499_CHAN_SINGLE_8P;
          adjacentChannel = LTC2499_CHAN_SINGLE_9P;
          break;
          
       case 9:
          newChannel = LTC2499_CHAN_SINGLE_9P;
          adjacentChannel = LTC2499_CHAN_SINGLE_10P;
          break;

       case 10:
          newChannel = LTC2499_CHAN_SINGLE_10P;
          adjacentChannel = LTC2499_CHAN_SINGLE_11P;
          break;

       case 11:
          newChannel = LTC2499_CHAN_SINGLE_11P;
          adjacentChannel = LTC2499_CHAN_SINGLE_12P;
          break;

       case 12:
          newChannel = LTC2499_CHAN_SINGLE_12P;
          adjacentChannel = LTC2499_CHAN_SINGLE_13P;
          break;

       case 13:
          newChannel = LTC2499_CHAN_SINGLE_13P;
          adjacentChannel = LTC2499_CHAN_SINGLE_14P;
          break;

       case 14:
          newChannel = LTC2499_CHAN_SINGLE_14P;
          adjacentChannel = LTC2499_CHAN_SINGLE_15P;
          break;

       case 15:
          newChannel = LTC2499_CHAN_SINGLE_15P;
          adjacentChannel = LTC2499_CHAN_SINGLE_0P;  // Bogus
          break;

       case 16:
       default:
          newChannel = LTC2499_CHAN_TEMPERATURE;
          break;

    }

    long adc = 0;
    ard2499board1.ltc2499ChangeChannel(newChannel);

    if (i > 15)
    {
      Serial.print("LTC2499 Temperature");
      Serial.print(" = [");
      Serial.print(ard2499board1.ltc2499ReadTemperature(ARD2499_TEMP_F));
      Serial.print(" F]\n");

      ard2499board1.ltc2499ChangeChannel(LTC2499_CHAN_SINGLE_0P);
      adc = ard2499board1.ltc2499Read();
      Serial.print("Channel 0 Voltage = ");
      Serial.print(" = [");
      Serial.print((VREF/2.0) * (adc / 16777216.0), 3);
      Serial.print(" V] (Expect ~ 1.625V)");

      float voltage = ard2499board1.ltc2499ReadVoltage();
      Serial.print(" [");
      Serial.print(ard2499board1.ltc2499ReadVoltage(), 3);
      Serial.print(" V]\n");

      i = 15;  // Stay on temperature
      delay(1000);
    }
    else
    {
      uint8_t count = 0;
      Serial.print("Channel ");
      Serial.print(i);
      do
      {
        adc = ard2499board1.ltc2499Read();
//        Serial.println(adc);
        if((-500 < adc) && (adc < 500))
        {
          count++;
          Serial.print("+");
        }
        else
        {
          count = 0;
          Serial.print(".");
        }
      } while(count < 3);

      // Check that it's not shorted to the adjacent channel
      ard2499board1.ltc2499ChangeChannel(adjacentChannel);

      count = 0;
      do
      {
        adc = ard2499board1.ltc2499Read();
//        Serial.println(adc);
        if((adc < -500) || (500 < adc))
        {
          count++;
          Serial.print("|");
        }
        else
        {
          count = 0;
          Serial.print("-");
        }
      } while(count < 3);

      // Verify it's still shorted to original channel
      ard2499board1.ltc2499ChangeChannel(newChannel);

      count = 0;
      do
      {
        adc = ard2499board1.ltc2499Read();
//        Serial.println(adc);
        if((-500 < adc) && (adc < 500))
        {
          count++;
          Serial.print("+");
        }
        else
        {
          count = 0;
          Serial.print(".");
        }
      } while(count < 1);
      
      Serial.println(" Done!");
    }
  }
}
*/


