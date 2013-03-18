/*************************************************************************
Title:    MRBus Arduino Library Header
Authors:  Nathan Holmes <maverick@drgw.net>, Colorado, USA
          Michael Petersen <railfan@drgw.net>, Colorado, USA
          Michael Prader, South Tyrol, Italy
File:     mrbus-avr.h
License:  GNU General Public License v3

LICENSE:
    Copyright (C) 2012 Nathan Holmes, Michael Petersen, and Michael Prader

    Original MRBus code developed by Nathan Holmes for PIC architecture.
    This file is based on AVR port by Michael Prader.  Updates and
    compatibility fixes by Michael Petersen.
    
    UART code derived from AVR UART library by Peter Fleury, and as
    modified by Tim Sharpe.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License along
    with this program. If not, see http://www.gnu.org/licenses/
    
*************************************************************************/
#ifndef ARD2499_H
#define ARD2499_H

#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <Wire.h>

#define FAKE_CONFIG1_TEMPERATURE   6
#define LTC2499_CONFIG1_ENABLE     5
#define LTC2499_CONFIG1_SINGLE_END 4
#define LTC2499_CONFIG1_ODD        3
#define LTC2499_CONFIG1_A2         2
#define LTC2499_CONFIG1_A1         1
#define LTC2499_CONFIG1_A0         0

#define LTC2499_CHAN_DIFF_0P_1N    (0)
#define LTC2499_CHAN_DIFF_2P_3N    (_BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_DIFF_4P_5N    (_BV(LTC2499_CONFIG1_A1))
#define LTC2499_CHAN_DIFF_6P_7N    (_BV(LTC2499_CONFIG1_A1) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_DIFF_8P_9N    (_BV(LTC2499_CONFIG1_A2))
#define LTC2499_CHAN_DIFF_10P_11N  (_BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_DIFF_12P_13N  (_BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A1))
#define LTC2499_CHAN_DIFF_14P_15N  (_BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A1) | _BV(LTC2499_CONFIG1_A0))

#define LTC2499_CHAN_DIFF_1P_0N    (_BV(LTC2499_CONFIG1_ODD))
#define LTC2499_CHAN_DIFF_3P_2N    (_BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_DIFF_5P_4N    (_BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A1))
#define LTC2499_CHAN_DIFF_7P_6N    (_BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A1) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_DIFF_9P_8N    (_BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A2))
#define LTC2499_CHAN_DIFF_11P_10N  (_BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_DIFF_13P_12N  (_BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A1))
#define LTC2499_CHAN_DIFF_15P_14N  (_BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A1) | _BV(LTC2499_CONFIG1_A0))

#define LTC2499_CHAN_SINGLE_0P     (_BV(LTC2499_CONFIG1_SINGLE_END))
#define LTC2499_CHAN_SINGLE_1P     (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_ODD))
#define LTC2499_CHAN_SINGLE_2P     (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_SINGLE_3P     (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_SINGLE_4P     (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_A1))
#define LTC2499_CHAN_SINGLE_5P     (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A1))
#define LTC2499_CHAN_SINGLE_6P     (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_A1) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_SINGLE_7P     (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A1) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_SINGLE_8P     (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_A2))
#define LTC2499_CHAN_SINGLE_9P     (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A2))
#define LTC2499_CHAN_SINGLE_10P    (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_SINGLE_11P    (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_SINGLE_12P    (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A1))
#define LTC2499_CHAN_SINGLE_13P    (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A1))
#define LTC2499_CHAN_SINGLE_14P    (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A1) | _BV(LTC2499_CONFIG1_A0))
#define LTC2499_CHAN_SINGLE_15P    (_BV(LTC2499_CONFIG1_SINGLE_END) | _BV(LTC2499_CONFIG1_ODD) | _BV(LTC2499_CONFIG1_A2) | _BV(LTC2499_CONFIG1_A1) | _BV(LTC2499_CONFIG1_A0))

// The temperature channel is actually fake - we manipulate things internally
#define LTC2499_CHAN_TEMPERATURE   (_BV(FAKE_CONFIG1_TEMPERATURE))

#define LTC2499_CONFIG2_ENABLE2     7
#define LTC2499_CONFIG2_IM          6
#define LTC2499_CONFIG2_FA          5
#define LTC2499_CONFIG2_FB          4
#define LTC2499_CONFIG2_SPD         3

#define LTC2499_CONFIG2_50HZ_REJ    (_BV(LTC2499_CONFIG2_FB))
#define LTC2499_CONFIG2_60HZ_REJ    (_BV(LTC2499_CONFIG2_FA))
#define LTC2499_CONFIG2_60_50HZ_REJ (0)  
#define LTC2499_CONFIG2_SPEED_2X    (_BV(LTC2499_CONFIG2_SPD))

class Ard2499
{
	public:
		void Ard2499();
		void begin(byte ltc2499Address, byte eepromAddress);
		const char* eui48Get();
		byte eepromRead(byte address);
		byte eepromWrite(byte address, byte blocking=0);
		unsigned long ltc2499Read();
		byte ltc2499ChangeChannel(byte channel);
		byte ltc2499ChangeConfiguration(byte config);
		unsigned int ltc2499ReadTemperatureK();
	private:
		byte i2cAddr_ltc2499;
		byte i2cAddr_eeprom;
		byte current2499Config;
		byte current2499Channel;
		char eui48[6*2+1];

};

#define LTC2499_READERROR 0xFFFFFFFF

unsigned long Ard2499::ltc2499Read()
{
	unsigned long retval=0;
	Wire.requestFrom(ltc2499Address, 4);
	// Error occurred, we don't have as many bytes as expected
	if (Wire.available() < 4)
		return(LTC2499_READERROR);

	retval |= Wire.read();
	retval <<= 8;
	retval |= Wire.read();
	retval <<= 8;
	retval |= Wire.read();
	retval <<= 8;
	retval |= Wire.read();

	return(retval);
}

unsigned int Ard2499::ltc2499ReadTemperatureK()
{
	unsigned long readVal = ltc2499Read();
	unsigned int tempK = 0;
	
	if (LTC2499_READERROR == readVal)
		return(0);

	// Throw away the sub-LSBs
	tempK = ((0x00FFFFFF & (readVal>>6)) * 4096) / 1570000;
	return(tempK);
}
#define ARD2499_INIT_SUCCESS      0
#define ARD2499_INIT_LTC2499_ERR  1
#define ARD2499_INIT_EEPROM_ERR   2

#define ARD2499_EEPROM_ADDR_EUI48  0xFA

byte Ard2499::begin(byte ltc2499Address, byte eepromAddress)
{
	byte retval = 0;
	byte i;
	
	memset(eui48, 0, sizeof(eui48));
	
	i2cAddr_ltc2499 = ltc2499Address;
	current2499Channel = LTC2499_CHAN_DIFF_0P_1N;
	current2499Config = LTC2499_CONFIG2_60_50HZ_REJ;
	
	Wire.beginTransmission(i2cAddr_ltc2499);
	Wire.write(0x80 | _BV(LTC2499_CONFIG1_ENABLE) | (0x1F & current2499Channel));
	Wire.write(_BV(LTC2499_CONFIG2_ENABLE) | (0x7F & current2499Config));
	retval = Wire.endTransmission(true);
	// Anything but zero means we couldn't initialize the LTC2499
	if (0 != retval)
		return(ARD2499_INIT_LTC2499_ERR);

	i2cAddr_eeprom = eepromAddress;

	Wire.beginTransmission(i2cAddr_eeprom);
	Wire.write(ARD2499_EEPROM_ADDR_EUI48);
	retval = Wire.endTransmission(false);
	// Anything but zero means we couldn't initialize the LTC2499
	if (0 != retval)
	{
		// Make sure we send a stop bit
		Wire.endTransmission(true);
		return(ARD2499_INIT_EEPROM_ERR);
	}

	Wire.requestFrom(i2cAddr_eeprom, 6, true);
	if (Wire.available() < 6)
		return(ARD2499_INIT_EEPROM_ERR);
	
	for(i=0; i<12; i+=2)
		sprintf(&eui48[i], "%02X", Wire.read());
		
	return(ARD2499_INIT_SUCCESS);
}

byte Ard2499::eepromRead(byte address)
{
	Wire.beginTransmission(i2cAddr_eeprom);
	Wire.write(address);
	retval = Wire.endTransmission(false);
	// Anything but zero means we couldn't initialize the LTC2499
	if (0 != retval)
	{
		// Make sure we send a stop bit
		Wire.endTransmission(true);
		return(ARD2499_INIT_EEPROM_ERR);
	}

	Wire.requestFrom(i2cAddr_eeprom, 1, true);
	if (Wire.available() < 1)
		return(0);
	return(Wire.read());
}

byte Ard2499::ltc2499ChangeChannel(byte channel)
{
	if (

	return(channel);
}

byte Ard2499::ltc2499ChangeConfiguration(byte config)
{
	return(config);	
}


#endif // MRBUS_AVR_H


