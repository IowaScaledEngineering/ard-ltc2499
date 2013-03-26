#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "ard2499.h"

unsigned long Ard2499::ltc2499Read()
{
	unsigned long retval=0;
	Wire.requestFrom((uint8_t)i2cAddr_ltc2499, (uint8_t)4);
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

Ard2499::Ard2499()
{
	init_status = ARD2499_INIT_LTC2499_ERR | ARD2499_INIT_EEPROM_ERR;
	i2cAddr_ltc2499 = 0;
	i2cAddr_eeprom = 0;
	current2499Config = 0;
	current2499Channel = 0;
	strcpy(eui48, "Unknown");
}

const char* Ard2499::eui48Get()
{
	return(eui48);
}


byte Ard2499::begin(byte ltc2499Address, byte eepromAddress)
{
	byte retval = 0;
	byte i;
	
	init_status = ARD2499_INIT_SUCCESS;
	strcpy(eui48, "Unknown");
	
	i2cAddr_ltc2499 = ltc2499Address;
	current2499Channel = LTC2499_CHAN_DIFF_0P_1N;
	current2499Config = LTC2499_CONFIG2_60_50HZ_REJ;
	
	Wire.beginTransmission(i2cAddr_ltc2499);
	Wire.write(0x80 | _BV(LTC2499_CONFIG1_ENABLE) | (0x1F & current2499Channel));
	Wire.write(_BV(LTC2499_CONFIG2_ENABLE2) | (0x7F & current2499Config));
	retval = Wire.endTransmission(true);
	// Anything but zero means we couldn't initialize the LTC2499
	if (0 != retval)
		init_status |= ARD2499_INIT_LTC2499_ERR;

	i2cAddr_eeprom = eepromAddress;
	Wire.beginTransmission(i2cAddr_eeprom);
	Wire.write(ARD2499_EEPROM_ADDR_EUI48);
	retval = Wire.endTransmission(false);
	// Anything but zero means we couldn't initialize the LTC2499
	if (0 != retval)
	{
		// Make sure we send a stop bit
		Wire.endTransmission(true);
		i2cAddr_eeprom = 0;
		init_status |= ARD2499_INIT_EEPROM_ERR;
	}
	else
	{

		Wire.requestFrom(i2cAddr_eeprom, (uint8_t)6, (uint8_t)true);
		if (Wire.available() < 6)
		{
			i2cAddr_eeprom = 0;		
			init_status |= ARD2499_INIT_EEPROM_ERR;
		} else {
			memset(eui48, 0, sizeof(eui48));
			for(i=0; i<12; i+=2)
				sprintf(&eui48[i], "%02X", Wire.read());
		}
	}		
	return(init_status);
}

byte Ard2499::eepromRead(byte address)
{
	byte retval = 0;
	Wire.beginTransmission(i2cAddr_eeprom);
	Wire.write(address);
	retval = Wire.endTransmission(false);
	// Anything but zero means we couldn't initialize the LTC2499
	if (0 != retval)
	{
		// Make sure we send a stop bit
		Wire.endTransmission(true);
		return(0);
	}

	Wire.requestFrom((uint8_t)i2cAddr_eeprom, (uint8_t)1, (uint8_t)true);
	if (Wire.available() < 1)
		return(0);
	return(Wire.read());
}

byte Ard2499::eepromWrite(byte address, byte value, byte blocking=1)
{
	uint8_t waitLoop = 10;
	byte retval = 0;
	if (0 == i2cAddr_eeprom)
		return(ARD2499_INIT_EEPROM_ERR);
		
	Wire.beginTransmission(i2cAddr_eeprom);
	Wire.write(address);
	Wire.write(value);
	retval = Wire.endTransmission(true);
	// Anything but zero means we couldn't write to the LTC2499
	if (0 != retval)
	{
		return(ARD2499_INIT_EEPROM_ERR);
	}	
	
	if (0 != blocking)
	{
		while (0 != waitLoop--)
		{
			Wire.beginTransmission(i2cAddr_eeprom);
			Wire.write(address);
			if (0 == Wire.endTransmission(true))
				return(0);
			_delay_ms(1);
		}
		return(ARD2499_INIT_EEPROM_ERR);
	}
	return(0);
}

byte Ard2499::ltc2499ChangeChannel(byte channel)
{
	return(channel);
}

byte Ard2499::ltc2499ChangeConfiguration(byte config)
{
	return(config);	
}

