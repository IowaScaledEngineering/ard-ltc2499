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

#define REG_PIN    0
#define VREF_PIN   1

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

Ard2499 ard2499board1;

byte confChan=0;

void setup()
{
	pinMode(GREEN_LED, OUTPUT);
	pinMode(RED_LED, OUTPUT);
	
	// initialize serial communications at 115200 bps:
	Serial.begin(115200);
	while(!Serial);
	Wire.begin();
	
	// Configure Arduino ADC
	analogReference(DEFAULT);
}

byte i=0;
byte fail = 0;

void loop()
{
	bool result = false;
	int adc = 0;
	float voltage = 0.0;
	uint8_t retval;

	float va = 2.5;
	float vb = 1.25;
	float deltaV = va - vb;

	fail = 0;
	
	Serial.print("VREF = ");
	Serial.print(VREF, 3);
	Serial.print("V\n\n");

	Serial.println("\nBasic Tests");

	// Regulator Test
	// +/-3% limits
	adc = analogRead(REG_PIN);
	voltage = (5.0 * adc / 1024.0) * 2;
	Serial.print("RegV: ");
	Serial.print(voltage, 3);
	Serial.print("V ... ");
	result = ((voltage > 4.85) && (voltage < 5.15));
	testResult(result);

	// LT6654 Test (crude pass/fail, not accuracy)
	// +/-1% limits
	adc = analogRead(VREF_PIN);
	voltage = (5.0 * adc / 1024.0);
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

	Serial.print("Looking for 0x50 (xxx)... ");
	result = findI2CSlave(0x50);
	testResult(result);

	Serial.print("Looking for 0x51 (A0)... ");
	result = findI2CSlave(0x51);
	testResult(result);

	Serial.print("Looking for 0x53 (A1)... ");
	result = findI2CSlave(0x53);
	testResult(result);

	Serial.print("Looking for 0x16 (CA0 L)... ");
	result = findI2CSlave(0x16);
	testResult(result);

	Serial.print("Looking for 0x15 (CA0 H)... ");
	result = findI2CSlave(0x15);
	testResult(result);

	Serial.print("Looking for 0x27 (CA1 L)... ");
	result = findI2CSlave(0x27);
	testResult(result);

	Serial.print("Looking for 0x24 (CA1 H)... ");
	result = findI2CSlave(0x24);
	testResult(result);

	Serial.print("Looking for 0x66 (CA2 L)... ");
	result = findI2CSlave(0x66);
	testResult(result);

	Serial.print("Looking for 0x45 (CA2 H)... ");
	result = findI2CSlave(0x45);
	testResult(result);

	ard2499board1.begin(ARD2499_ADC_ADDR_ZZZ, ARD2499_EEP_ADDR_ZZ, VREF * 1000.0);
	ard2499board1.ltc2499ChangeConfiguration(LTC2499_CONFIG2_60_50HZ_REJ);
	ard2499board1.ltc2499ChangeChannel(LTC2499_CHAN_SINGLE_0P);

	Serial.print("eeprom mac = [");
	Serial.print(ard2499board1.eui48Get());
	Serial.print("]\n");

	Serial.print("write eeprom[0] 0x");
	Serial.print(0xAA, HEX);
	retval = ard2499board1.eepromWrite(0, 0xAA, true);
	Serial.print(" retval=");
	Serial.print(retval);
	Serial.print("\n");

	Serial.print("read eeprom[0] 0x");
	Serial.print(ard2499board1.eepromRead(0, true), HEX);
	Serial.print("\n");

	Serial.print("write eeprom[0] 0x");
	Serial.print(0x55, HEX);
	retval = ard2499board1.eepromWrite(0, 0x55, true);
	Serial.print(" retval=");
	Serial.print(retval);
	Serial.print("\n");

	Serial.print("read eeprom[0] 0x");
	Serial.print(ard2499board1.eepromRead(0, true), HEX);
	Serial.print("\n");

	delay(1000);

skipJumpers:

	// LTC2499 Analog Input Test
	// +/-0.1% limits
	Serial.println("\nADC Analog Input");

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
	voltage *= -1;
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH2-CH3:   ");
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_4P_5N);
	voltage *= -1;
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH4-CH5:   ");
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_6P_7N);
	voltage *= -1;
	Serial.print(voltage, 6);
	Serial.print("V ... ");
	result = ((voltage > (deltaV*0.999)) && (voltage < (deltaV*1.001)));
	testResult(result);
	
	Serial.print("ADC CH6-CH7:   ");
	voltage = ard2499board1.ltc2499ReadVoltageAndChangeChannel(LTC2499_CHAN_DIFF_8P_9N);
	voltage *= -1;
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
		Serial.println(ANSI_COLOR_RED "********************");
		Serial.println(" FAILED...");
		Serial.println("********************" ANSI_COLOR_RESET);
	}
	else
	{
		Serial.println(ANSI_COLOR_GREEN "********************");
		Serial.println(" PASSED!");
		Serial.println("   INSTALL JUMPERS!");
		Serial.println("********************" ANSI_COLOR_RESET);
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
		Serial.println(ANSI_COLOR_GREEN "Pass!" ANSI_COLOR_RESET);
	}
	else
	{
		Serial.println(ANSI_COLOR_RED "*** Fail ***" ANSI_COLOR_RESET);
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

uint8_t findI2CSlave(uint8_t addr)
{
	byte stat, done = 0;
	uint8_t i;
	while(!done)
	{
		stat = 0;
		Wire.beginTransmission(addr);
		for(i=0; i<3; i++)
		{
			stat = Wire.endTransmission();
			if(stat)
			{
				i = 0;
			}
			delay(150);
		}
		if(!stat)
		{
			Serial.print("\a");
			done = 1;
		}
	}
	return done;
}


