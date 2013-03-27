#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "Ard2499.h"

byte Ard2499::ltc2499ChangeChannel(byte channel)
{
	return(ltc2499ChangeChannel(channel, true));
}

byte Ard2499::ltc2499ChangeChannel(byte channel, bool addStop)
{
	uint8_t config1=0, config2=0, blockingCountdown;	
	
	if (LTC2499_CHAN_TEMPERATURE == channel)
	{
		// The temperature channel is fake - it really has to change the config byte in the message
		current2499Channel = LTC2499_CHAN_TEMPERATURE;
		config1 = 0x80 | _BV(LTC2499_CONFIG1_ENABLE);
		config2 = _BV(LTC2499_CONFIG2_ENABLE2) | _BV(LTC2499_CONFIG2_IM) | (LTC2499_CONFIG2_CONFBITS & current2499Config);
	}
	else
	{
		current2499Channel = LTC2499_CONFIG1_CHANBITS & channel;
		config1 = 0x80 | _BV(LTC2499_CONFIG1_ENABLE) | (0x1F & current2499Channel);
		config2 = _BV(LTC2499_CONFIG2_ENABLE2) | (LTC2499_CONFIG2_CONFBITS & current2499Config);
	}

	// If there's no ltc2499 address, error out
	if(0 == i2cAddr_ltc2499)
		return(ARD2499_LTC2499_ERR);

	// Block in 5ms increments for up to 200ms.  Given the conversion rate 
	// should be greater than 7.5sps, 200ms is more than we should ever have to
	// wait.

	blockingCountdown = 40;
	while(blockingCountdown--)
	{
		Wire.beginTransmission(i2cAddr_ltc2499);
		Wire.write(config1);
		Wire.write(config2);
		if (0 != Wire.endTransmission(addStop))
		{
			if (0 == blockingCountdown)
				return(ARD2499_LTC2499_ERR);
			_delay_ms(5);
		}	
		else
			break;
	}

	return(ARD2499_SUCCESS);
}

byte Ard2499::ltc2499ChangeConfiguration(byte config)
{
	current2499Config = LTC2499_CONFIG2_CONFBITS & config;
	return(ltc2499ChangeChannel(current2499Channel));
}

long Ard2499::ltc2499Read()
{
	unsigned long rawValue = ltc2499ReadRaw();
	
	if (LTC2499_RAW_READ_ERROR == rawValue)
		return(LTC2499_READ_ERROR);

	uint8_t upperByte = 0xFF & (rawValue>>24);
	switch(upperByte)
	{
		case 0xC0:
			return(LTC2499_OVERRANGE_NEGATIVE);
		case 0x3F:
			return(LTC2499_OVERRANGE_POSITIVE);
	}
	rawValue = (rawValue & 0x7FFFFFFF)>>6; // Get rid of the sub-LSBs

	if (0x01000000 & rawValue)
		rawValue |= 0xFF000000;
	return((long)rawValue);

}

long Ard2499::ltc2499ReadAndChangeChannel(byte nextChannel)
{
	if (ARD2499_SUCCESS != ltc2499ChangeChannel(nextChannel, false))
	{
		// Run another transmission through just to send out a stop bit
		Wire.beginTransmission(i2cAddr_ltc2499);
		Wire.endTransmission((uint8_t)true);
		return(LTC2499_READ_ERROR);
	}
	// Otherwise, we've succeeded and are sitting on a restart condition
	return(ltc2499Read());
}

unsigned long Ard2499::ltc2499ReadRaw()
{
	unsigned long retval=0;
	uint8_t blockingCountdown=40;

	if (0 == i2cAddr_ltc2499)
		return(0);

	// Block in 5ms increments for up to 200ms.  Given the conversion rate 
	// should be greater than 7.5sps, 200ms is more than we should ever have to
	// wait.
	while(blockingCountdown--)
	{
		Wire.requestFrom((uint8_t)i2cAddr_ltc2499, (uint8_t)4, (uint8_t)true);
		// Error occurred, we don't have as many bytes as expected
		if (Wire.available() < 4)
		{
			if (0 == blockingCountdown)
				return(LTC2499_RAW_READ_ERROR);
			_delay_ms(5);
		}	
		else
			break;
	}

	retval |= Wire.read();
	retval <<= 8;
	retval |= Wire.read();
	retval <<= 8;
	retval |= Wire.read();
	retval <<= 8;
	retval |= Wire.read();

	return(retval);
}

unsigned long Ard2499::ltc2499ReadRawAndChangeChannel(byte nextChannel)
{
	if (ARD2499_SUCCESS != ltc2499ChangeChannel(nextChannel, false))
	{
		// Run another transmission through just to send out a stop bit
		Wire.beginTransmission(i2cAddr_ltc2499);
		Wire.endTransmission((uint8_t)true);
		return(LTC2499_RAW_READ_ERROR);
	}
	// Otherwise, we've succeeded and are sitting on a restart condition
	return(ltc2499ReadRaw());
}

float Ard2499::ltc2499ReadTemperature(byte temperatureUnits)
{
	unsigned int tempDK = ltc2499ReadTemperatureDeciK();
	float tempK = (float)tempDK/10.0;
	switch(temperatureUnits)
	{
		case ARD2499_TEMP_K:
			return(tempK);
		case ARD2499_TEMP_C:
			return(tempK - 273.15);
		case ARD2499_TEMP_F:
			return(((tempK - 273.15) * 9.0)  / 5.0 + 32.0);
	}
	return(0);
}

unsigned int Ard2499::ltc2499ReadTemperatureDeciK()
{
	unsigned long readVal = 0;
	unsigned int tempDK = 0;
	
	// If we're currently not set for the temperature channel, switch us over
	// If we are currently set for the temp channel, then that write will have
	// triggered a conversion, and ltc2499ReadRaw will block until it's done
	if(LTC2499_CHAN_TEMPERATURE != current2499Channel)
		ltc2499ChangeChannel(LTC2499_CHAN_TEMPERATURE, true);
	
	readVal = ltc2499ReadRaw();
	
	if (LTC2499_RAW_READ_ERROR == readVal)
		return(0);

	// Throw away the sub-LSBs
	readVal >>= 6;
	tempDK = ((0x00FFFFFF & readVal) * 256) / 19625;

	// tempDK is now the temperature in deci-kelvin
	return(tempDK);
}

Ard2499::Ard2499()
{
	init_status = ARD2499_LTC2499_ERR | ARD2499_EEPROM_ERR;
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
	
	init_status = ARD2499_SUCCESS;
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
	{
		i2cAddr_ltc2499 = 0;
		init_status |= ARD2499_LTC2499_ERR;
	}

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
		init_status |= ARD2499_EEPROM_ERR;
	}
	else
	{

		Wire.requestFrom(i2cAddr_eeprom, (uint8_t)6, (uint8_t)true);
		if (Wire.available() < 6)
		{
			i2cAddr_eeprom = 0;		
			init_status |= ARD2499_EEPROM_ERR;
		} else {
			memset(eui48, 0, sizeof(eui48));
			for(i=0; i<12; i+=2)
				sprintf(&eui48[i], "%02X", Wire.read());
		}
	}		
	return(init_status);
}

byte Ard2499::eepromRead(int address, byte defaultOnError=0)
{
	byte retval = 0;
	
	// Our address range only goes to 0xFF
	if (address > 0xFF)
		return(defaultOnError);
	
	Wire.beginTransmission(i2cAddr_eeprom);
	Wire.write((uint8_t)address);
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
		return(defaultOnError);
	return(Wire.read());
}

byte Ard2499::eepromWrite(int address, byte value, byte blocking=1)
{
	uint8_t waitLoop = 10;
	byte retval = 0;
	if (0 == i2cAddr_eeprom)
		return(ARD2499_EEPROM_ERR);

	// Our writable address range only goes to 0xFF
	if (address > 0x7F)
		return(ARD2499_EEPROM_ERR);
		
	Wire.beginTransmission(i2cAddr_eeprom);
	Wire.write(address);
	Wire.write(value);
	retval = Wire.endTransmission(true);
	// Anything but zero means we couldn't write to the LTC2499
	if (0 != retval)
	{
		return(ARD2499_EEPROM_ERR);
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
		return(ARD2499_EEPROM_ERR);
	}
	return(ARD2499_SUCCESS);
}


