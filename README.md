# Iowa Scaled Engineering ARD-LTC2499

## Summary

The [ARD-LTC2499](https://www.iascaled.com/store/ARD-LTC2499) is an Arduino-compatible shield that contains a Linear Technology LTC2499 24-bit ADC
coupled with an LT6654 precision voltage reference. It is capable of converting 16 single-ended channels,
8 differential channels, or any other combination. In addition to measuring voltage, the ADC can interface
directly to a variety of sensors including strain gauges, thermocouples, and current shunts. The onboard EEPROM
can be used to store calibration and configuration information directly on each ARD-LTC2499 board.
A 6-byte EUI-48-compatible globally unique ID number is also provided.  The ARD-LTC2499 can be used with other
Arduino shields to make a simple, yet very accurate, data acquisition system.

Note:  The normal ARD-LTC2499 is not compatible with Arduinos using 3.3V I/O on the I2C lines, such as the Due, Zero, etc.
This is because the LTC2499 requires a 5V supply to deal with the 4.096V reference voltage, making it incompatible with 3.3V I2C.
A 3.3V compatible version is [available as the ARD-LTC2499-3V3](https://www.iascaled.com/store/ARD-LTC2499-3V3), which
is identical except that it also includes on-board level shifters for the I2C lines.

### Features

* Precision 24-bit delta sigma ADC in an Arduino-friendly form factor
* 16 single-ended channels, 8 differential channels, or any combination
* 7.5 samples/sec (or 15 samples/second without automatic offset cancellation)
* Input common mode voltage range of -0.3V to 5.3V
* Input measurement range of 0-2.048V (single-ended) or -2.048V to 2.048V (differential)
* 4.096V LT6654 precision voltage reference on board
* Optional onboard 5V linear regulator provides clean power to the ADC and reference
* 128 bytes of onboard EEPROM for storing configuration or calibration values
* Read-only 6-byte EUI-48-compatible globally unique ID
* Arduino software library for easy use
* 5V compatible I2C interface (or 3.3-5V with the ARD-LTC2499-3V3)


## Operation of the ARD-LTC2499 Hardware

The latest schematics for both the ARD-LTC2499 and the ARD-LTC2499-3V3 can be found here:
* [ARD-LTC2499 v1.3](https://github.com/IowaScaledEngineering/ard-ltc2499/raw/master/pg/ard-ltc2499-v1.3-83eb2df/ard-ltc2499.pdf)
* [ARD-LTC2499-3V3 v1.3](https://github.com/IowaScaledEngineering/ard-ltc2499/raw/master/pg/ard-ltc2499-v1.3LS-ffd4822/ard-ltc2499.pdf)

The Golden Rule:  When in doubt, check [the LTC2499
datasheet](https://www.analog.com/media/en/technical-documentation/data-sheets/2499fe.pdf).  This guide is not
designed to be a comprehensive guide to the LTC2499.

The useful measuring range of the inputs is 0V to 2.048V in single-ended mode, and a +/-2.048V in differential mode.
Be careful of common mode limits, however. Inputs have an absolute maximum common mode limit of -0.3V to +0.3V above VCC (nominally 5V)
before damage to the part will occur, so while the differential inputs can have -2.048V between them, neither side of the input may
violate the limit of -0.3V to 5.3V without damaging the part.

To eliminate any possibility of introducing gain or offset errors from buffering, the pins of the LTC2499 are brought directly
to the terminal headers. Please be careful when handling and using the board. Errant electrostatic discharge or voltages applied
outside the absolute maximum limits specified in the datasheet can cause immediate damage.

The 4.096V Reference output on J3 comes directly from the LT6654 reference output with no buffering. The LT6654 is rated for a
maximum of +/- 10mA. Also note that any current drawn from this pin will degrade the accuracy of the reference by a
small amount (see the Electrical Characteristics table, "Load Regulation", in the LT6654 datasheet).

### Hardware Configuration

In order to adapt to a wide variety of use cases and ArduinoTM configurations, the ARD-LTC2499 board has a number of configuration jumpers.

* JP1 - I2C Pull-up Enable
* JP2 - I2C Pin Selection
* JP3 - Onboard 5V Linear Regulator Enable
* JP4, JP5 & JP6 - LTC2499 I2C Address
* JP7 & JP8 - EEPROM I2C Address


_JP1 - I2C Pull-up Enable_

The I2C bus depends upon a set of pull-up resistors to function correctly. Each I2C bus should have one and only one set of
pull-ups enabled. The ARD-LTC2499 includes a set of 1.6k pull-up resistors that can be enabled by jumpering pins 1-2 and pins 3-4 of JP1.
Note that Arduino boards do not typically provide I2C pull-ups, so at least one board connected to I2C must provide them.

_JP2 - I2C Pin Selection_

Not all Arduinos provide I2C on the same pins. With the Rev 3 hardware designs (almost all of them as of 2021), there are now a
dedicated set of pins above AREF and GND marked "SDA" and "SCL". If you see those on your board, then place jumpers between
pins 3-5 and 4-6 on JP2 to connect the I2C bus to them.

If you board does not have explicit SDA and SCL pins, then most likely they're shared with pins A4 and A5. Place jumpers
between pins 1-3 and 2-4 to select those instead.

_JP3 - Onboard 5V Linear Regulator Enable_

To free the LTC2499 from any noise on the Arduino's 5V supply caused by the AVR or other shields, the ARD-LTC2499 includes an
onboard 78L05 regulator coming from the Arduino's "VIN" pin. If you are powering the Arduino with >8VDC via the power jack,
you can use the ARD-LTC2499 onboard 5V regulator to provide nice quiet power by jumpering pins 3-4 of JP3.

If you are powering the Arduino off of the 5V provided by USB or via some other means that does not place >8VDC on the VIN pin,
then the regulator will not work and must not be used. JP3 must be placed between pins 1-2 in this case.

_JP4, JP5 & JP6 - LTC2499 I2C Address_

Each device on the I2C bus must have a unique device address. The LTC2499 has 27 possible addresses depending on how
JP4, JP5, and JP6 are set. Each may be set high (jumpering pins 1-2, the two closest to the LTC2499), low (jumpering pins 3-4,
the ones furthest from the 2499) or floating (no jumpered pins or pins 2-3 jumpered).  JP4 corresponds to CA0, JP5 to CA1, and JP6 to CA2 on the LTC2499.

| JP4    | JP5    | JP6    | 7-bit I2C Address | Ard2499 Define       |
|--------|--------|--------|-------------------|----------------------|
| Low    | Low    | Low    | 0x14              | ARD2499_ADC_ADDR_000 |
| Low    | Low    | Open   | 0x15              | ARD2499_ADC_ADDR_00Z |
| Low    | Low    | High   | 0x16              | ARD2499_ADC_ADDR_001 |
| Low    | Open   | Low    | 0x17              | ARD2499_ADC_ADDR_0Z0 |
| Low    | Open   | Open   | 0x24              | ARD2499_ADC_ADDR_0ZZ |
| Low    | Open   | High   | 0x25              | ARD2499_ADC_ADDR_0Z1 |
| Low    | High   | Low    | 0x26              | ARD2499_ADC_ADDR_010 |
| Low    | High   | Open   | 0x27              | ARD2499_ADC_ADDR_01Z |
| Low    | High   | High   | 0x28              | ARD2499_ADC_ADDR_011 |
| Open   | Low    | Low    | 0x35              | ARD2499_ADC_ADDR_Z00 |
| Open   | Low    | Open   | 0x36              | ARD2499_ADC_ADDR_Z0Z |
| Open   | Low    | High   | 0x37              | ARD2499_ADC_ADDR_Z01 |
| Open   | Open   | Low    | 0x44              | ARD2499_ADC_ADDR_ZZ0 |
| Open   | Open   | Open   | 0x45              | ARD2499_ADC_ADDR_ZZZ |
| Open   | Open   | High   | 0x46              | ARD2499_ADC_ADDR_ZZ1 |
| Open   | High   | Low    | 0x47              | ARD2499_ADC_ADDR_Z10 |
| Open   | High   | Open   | 0x54              | ARD2499_ADC_ADDR_Z1Z |
| Open   | High   | High   | 0x55              | ARD2499_ADC_ADDR_Z11 |
| High   | Low    | Low    | 0x56              | ARD2499_ADC_ADDR_100 |
| High   | Low    | Open   | 0x57              | ARD2499_ADC_ADDR_10Z |
| High   | Low    | High   | 0x64              | ARD2499_ADC_ADDR_101 |
| High   | Open   | Low    | 0x65              | ARD2499_ADC_ADDR_1Z0 |
| High   | Open   | Open   | 0x66              | ARD2499_ADC_ADDR_1ZZ |
| High   | Open   | High   | 0x67              | ARD2499_ADC_ADDR_1Z1 |
| High   | High   | Low    | 0x74              | ARD2499_ADC_ADDR_110 |
| High   | High   | Open   | 0x75              | ARD2499_ADC_ADDR_11Z |
| High   | High   | High   | 0x76              | ARD2499_ADC_ADDR_111 |


_JP7 & JP8 - EEPROM I2C Address_

Each device on the I2C bus must have a unique device address. The 24AA025E48 EEPROM that provides both non-volatile storage
and the globally unique ID number has four possible addresses depending on how JP7 and JP8 are set. Each may be set high (no jumper)
or low (jumper in place). Please refer to the Figure 5-2 in the 24AA025E48 datasheet (we use the SOT-23 version, so A2 is always '0' -
otherwise JP8 is A1 and JP7 is A0) or the Ard2499 library header for corresponding address values.

| JP7    | JP8    | 7-bit I2C Address | Ard2499 Define      |
|--------|--------|-------------------|---------------------|
| Jumper | Jumper | 0x50              | ARD2499_EEP_ADDR_00 |
| Jumper | Open   | 0x51              | ARD2499_EEP_ADDR_0Z |
| Open   | Jumper | 0x52              | ARD2499_EEP_ADDR_Z0 |
| Open   | Open   | 0x53              | ARD2499_EEP_ADDR_ZZ |

## Ard2499 Arduino Software Library

### Introduction

The Ard2499 library was designed to make the Iowa Scaled Engineering ARD-LTC2499 shield easy to use with the Arduino environment.
The board itself is provides a breakout for the Linear Technology LTC2499 24-bit 8/16 channel delta-sigma ADC, adding other useful
features like a precision 4.096V LT6654 reference, 128 bytes of onboard EEPROM for storing configuration or calibration values,
and a read-only 6-byte EUI48-compatible globally unique ID number.

Note:  The software library is now maintained in [its own repository](https://github.com/IowaScaledEngineering/arduino-ard2499) for compatibility
with Arduino's Library Manager.

### Setup

There are two ways to install the Ard2499 library in the Arduino environment.

The easiest way is through Arduino's Library Manager.  You should be able to search for Ard2499 and have the library available for installation shortly.
We're working on it.

Alternately, you can [download the zip file](https://github.com/IowaScaledEngineering/arduino-ard2499/archive/main.zip) from the
IowaScaledEngineering/arduino-ard2499 project on Github and follow the ["Importing a .zip Library" instructions](https://www.arduino.cc/en/guide/libraries#toc4).

### Example Sketch

Once the library is installed, you should see Ard2499 under File->Examples in your Arduino environment.  A sample Arduino sketch that demonstrates some
simple uses of the library is available as ard2499_example directory.

### Library Usage Details

The core of the Ard2499 library is the Ard2499 object.  One instance should be instantiated for each ARD-LTC2499 board that you are using.

When an instance is created, an optional TwoWire object can be passed to the constructor.  Otherwise, the Ard2499 library assumes that the board will be communicating using the Wire object.

#### Functional Interface

```byte Ard2499::begin(byte ltc2499Address, byte eepromAddress, uint16_t referenceMillivolts = 4096);```

The Ard2499::begin() function should be called within the setup() function of your sketch. It requires the 7-bit address of both the LTC2499 and EEPROM on the ARD-LTC2499 to initialize both parts.
The addresses can be passed in directly as a number, or by using the address macros.  By using different addresses, it's possible to stack ARD-LTC2499 boards and have a different object to control each one.

An optional reference voltage, in millivolts, can be passed if the ARD-LTC2499 has a non-standard reference on board. If not specified, it defaults to 4096mV, which is the voltage
for any non-special order board.

Returns:  Ard2499::begin() will return a byte corresponding to a bitmast of errors, with 0 corresponding to complete success.
* ARD2499_SUCCESS (0x00) upon both devices being initialized successfully
* ARD2499_LTC2499_ERR (0x01) if the LTC2499 is not initialized successfully
* ARD2499_EEPROM_ERR (0x02) if the EEPROM is not initialized successfully

```byte Ard2499::ltc2499ChangeChannel(byte channel)```

The Ard2499::ltc2499ChangeChannel() function will wait for up to 200 milliseconds for the LTC2499 to become available, change the LTC2499's input multiplexer to the channel
specified in channel and start a new conversion. The value in channel corresponds to the lower 5 bits in the first configuration word of the device. In addition to computing
values for channel yourself, you can use the channel macros.

Additionally, channel can be configured to LTC2499_CHAN_TEMPERATURE to return the LTC2499 internal temperature. While the temperature input isn't a "channel" on the multiplexer,
the library makes it appear this way.

Returns:
* ARD2499_SUCCESS (0x00) if the channel change completed
* ARD2499_LTC2499_ERR (0x02) if the channel change failed

```byte Ard2499::ltc2499ChangeConfiguration(byte config)```

The Ard2499::ltc2499ChangeConfiguration() function will wait for up to 200 milliseconds for the LTC2499 to become available, change the LTC2499's configuration to that
specified in config and start a new conversion. The value in config corresponds to bits 3-5 in the second configuration word of the device. In addition to computing values
for config yourself, you can use the configuration macros.

Returns:
* ARD2499_SUCCESS (0x00) if the configuration change completed
* ARD2499_LTC2499_ERR (0x02) if the configuration change failed

```long Ard2499:: ltc2499Read()```

The Ard2499::ltc2499Read() function will wait for up to 200 milliseconds for the LTC2499 to become available, read the last conversion value, start a new conversion,
and clean up the conversion result.

Returns:
* -16777216 to +16777215 - normal conversion result
* LTC2499_READ_ERROR (0x01000001) if the read timed out or otherwise failed
* LTC2499_OVERRANGE_POSITIVE (0x01000000) if (IN+ - IN-) > 2.048V
* LTC2499_OVERRANGE_NEGATIVE (0x11000000) if (IN+ - IN-) < 2.048V

```long Ard2499:: ltc2499ReadAndChangeChannel(byte nextChannel)```

The Ard2499::ltc2499ReadAndChangeChannel() function will first attempt to change the input multiplexer channel and then perform a read. This is useful for not wasting
the conversion (and delay) resulting from a normal Ard2499::ltc2499Read() if the next operation is just going to be changing the input channel. Note: the channel change
does not take effect until after the data is read - in other words, for the next conversion.

Channel change behaviour works exactly like Ard2499::ltc2499ChangeChannel(). In the event that the channel change fails, LTC2499_READ_ERROR will be returned. Otherwise,
read and return behaviour works exactly like Ard2499::ltc2499Read().

```unsigned long Ard2499:: ltc2499ReadRaw()```

The Ard2499::ltc2499ReadRaw() function will wait for up to 200 milliseconds for the LTC2499 to become available, read the last conversion value, start a new conversion,
and return the native 32 bit conversion result. The lowest 6 bits will be "sub-LSB" bits, and the highest bit will be what the datasheet designates "SIG". This is for
advanced users who want direct access to the conversion result.

Returns:
* LTC2499_RAW_READ_ERROR (0xFFFFFFFF) if the read timed out or otherwise failed
* Any other result is a direct read from the LTC2499

```unsigned long Ard2499:: ltc2499ReadRawAndChangeChannel(byte nextChannel)```

The Ard2499::ltc2499ReadRawAndChangeChannel() function will first attempt to change the input multiplexer channel and then perform a raw read. This is useful for
not wasting the conversion (and delay) resulting from a normal Ard2499::ltc2499ReadRaw() if the next operation is just going to be changing the input channel.
Note: the channel change does not take effect until after the data is read - in other words, for the next conversion. The raw read capability is for advanced
users who want direct access to the conversion result.

Channel change behaviour works exactly like Ard2499::ltc2499ChangeChannel(). In the event that the channel change fails, LTC2499_RAW_READ_ERROR will be returned.
Otherwise, read and return behaviour works exactly like Ard2499:: ltc2499ReadRaw().

```unsigned int Ard2499:: ltc2499ReadTemperatureDeciK()```

The Ard2499::ltc2499ReadTemperatureDeciK() function will check if the current channel is LTC2499_CHAN_TEMPERATURE and if not, wait up to 200 milliseconds for
converter availability, change it and start a new conversion. It will then wait for up to 200 milliseconds for the LTC2499 to complete the conversion and turn it
into a unsigned int corresponding to the LTC2499's current temperature in 1/10th degrees Kelvin.

Upon failure, it will return zero, as hopefully no project you're working on is operating at absolute zero. If it is and the part still seems to be working
otherwise, call us and we'll help you write the paper for your physics journal of choice.

```float ltc2499ReadTemperature(byte temperatureUnits)```

The Ard2499::ltc2499ReadTemperature() function will check if the current channel is LTC2499_CHAN_TEMPERATURE and if not, wait up to 200 milliseconds for converter
availability, change it and start a new conversion. It will then wait for up to 200 milliseconds for the LTC2499 to complete the conversion and turn it into single
precision floating point value corresponding to the LTC2499's current temperature in the units specified in temperatureUnits.

Upon failure, it will return zero.

The value of temperatureUnits should be ARD2499_TEMP_K (degrees Kelvin), ARD2499_TEMP_C (degrees Celsius), or ARD2499_TEMP_F (degrees Fahrenheit).

Note: Because this function uses floating point on an 8 bit processor without any sort of hardware floating point support, expect performance to be abysmal on
AVR-based Arduinos. But it's handy if you don't care about performance.

```const char* Ard2499::eui48Get()```

The Ard2499::eui48Get() function will retrieve the board's 48-bit, globally unique, EUI-48TM-compatible identification number as a string.

Returns:

Ard2499::eui48Get() will return a character pointer to a constant, NULL-terminated string. The value will be either 12 characters of hexadecimal number,
or "Unknown" if the Ard2499::begin() failed to find the EEPROM. (Ard2499::begin() would have returned a value with the ARD2499_EEPROM_ERR bit set.)

```byte Ard2499::eepromRead(int address, byte defaultOnError=0x00)```

The Ard2499::eepromRead() function will retrieve a byte from the ARD-LTC2499's onboard EEPROM at the location specified by address. Valid values for address are 0x00-0x7F.

Optionally, the function can take a second argument - defaultOnError - of the value to be returned if the EEPROM is not initialized or is not accessible when the function
is called. If the defaultOnError parameter is omitted, it will default to returning 0x00 on failure.

```byte Ard2499::eepromWrite(int address, byte value, byte blocking=1)```

The Ard2499::eepromWrite() function will write a byte of value value to the ARD-LTC2499's onboard EEPROM at the location specified by address. Valid values for address are
0x00-0x7F, and value can be 0x00-0xFF.

Optionally, the function can take a third argument - blocking. The Microchip 24AA025E48 EEPROM used on the ARD-LTC2499 can take up to 5 milliseconds to actually commit
a write to EEPROM. If blocking is non-zero or omitted, the eepromWrite() function will block until it can confirm that the write is completed or until it times out
after 10 millseconds.

Note that blocking will not cause the value to be verified - it merely causes the function to block until the part begins acknowledging I2C/TWI transactions once
again, indicating that the write cycle is complete.

Returns:
* ARD2499_SUCCESS (0x00) if the write completed
* ARD2499_EEPROM_ERR (0x02) if the write failed


#### LTC2499 Address Defines

The address for the LTC2499 is configurable on the ARD-LTC2499 using jumpers JP4-JP6. You can either use the datasheet and directly enter the LTC2499's address into
the Ard2499::begin() function, or use some convenient macros.

The macros are all in the form of ARD2499_ADC_ADDR_cba, where a, b, and c can take on the value of '0', '1', or 'Z'. Placeholder 'a' represents jumper JP4, 'b' represents JP5,
and 'c' JP6. '0' indicates that the center pin is jumpered to the low pins (towards inputs 8-15 on the board), '1' indicates that it is jumpered to the high pins
(towards the LTC2499 or inputs 0-7), and 'Z' indicates that it's left floating (no jumpers or jumper across pin 2-3).

So, if all three were left open, you'd use ARD2499_ADC_ADDR_ZZZ. Likewise, for JP4 to bottom, JP5 to top, and JP6 open you'd use ARD2499_ADC_ADDR_Z10.

#### EEPROM Address Defines

The address for the EEPROM is configurable on the ARD-LTC2499 using jumpers JP7-JP8. You can either use the datasheet and directly enter the 24AA025E48's address into the
Ard2499::begin() function, or use some convenient macros.

The macros are all in the form of ARD2499_EEP_ADDR_ba, where a and b can take on the value of '0' or 'Z'. Placeholder 'a' represents jumper JP7 and 'b' represents JP8.
'0' indicates that a jumper has been placed across those pins, and 'Z' indicates that it's left floating (no jumper).

So, if both were left open, you'd use ARD2499_EEP_ADDR_ZZ. Likewise, for JP7 jumpered and JP8 open ARD2499_EEP_ADDR_Z0.

#### Channel Defines

The library provides definitions of all the common channel combinations to allow the easy selection of input channels without knowing all the internal LTC2499 bits to set.

Differential configurations take the form LTC2499_CHAN_DIFF_aP_bN, where 'a' represents the channel number of the positive input (IN+ in the LTC2499 datasheet)
and 'b' represents the channel number of the negative input (IN-).

Single-ended configurations are in the form LTC2499_CHAN_SINGLE_aP, where again 'a' represents the channel number of the positive (IN+) input.

Additionally, the internal temperature sensor can be selected by using LTC2499_CHAN_TEMPERATURE.

Some examples:
* LTC2499_CHAN_DIFF_2P_3N - Differential, input 2 is IN+, input 3 is IN-
* LTC2499_CHAN_DIFF_11P_10N - Differential, input 11 is IN+, input 10 is IN-
* LTC2499_CHAN_SINGLE_2P - Single ended, input 2 as IN+
* LTC2499_CHAN_SINGLE_10P - Single ended, input 10 as IN+
* LTC2499_CHAN_TEMPERATURE - Connect to internal temperature sensor.

#### Configuration Macros

Bit position macros are provided corresponding to the bits used to define the LTC2499 configuration in the second configuration byte:
* LTC2499_CONFIG2_FA
* LTC2499_CONFIG2_FB
* LTC2499_CONFIG2_SPD

These can be combined into a configuration value by an expression such as:

(_BV(LTC2499_CONFIG2_FA) | _BV(LTC2499_CONFIG2_SPD))

Please see the LTC2499 datasheet, Table 4, to see how these are used directly.

Additionally, the library has some convenient combinations:
* LTC2499_CONFIG2_50HZ_REJ - Reject at 50Hz
* LTC2499_CONFIG2_60HZ_REJ - Reject at 60Hz
* LTC2499_CONFIG2_60_50HZ_REJ - Reject at both 50/60Hz
* LTC2499_CONFIG2_SPEED_2X - Set for double speed conversions, drop offset calibration (15 samples/sec)

To use these, pick one of the rejection configurations and - optionally - bitwise or it with the double speed option and pass it to the the ltc2499ChangeConfiguration() function. For example:

```retval = ard2499.ltc2499ChangeConfiguration(LTC2499_CONFIG2_60_50HZ_REJ | LTC2499_CONFIG2_SPEED_2X)```

### A Note About Converter Latency

Important! Note that the LTC2499 will start a conversion whenever it is read (using ltc2499Read() or similar) or whenever the channel
or configuration is changed. Each conversion will take ~133 milliseconds in 1X mode and ~67 milliseconds in 2X mode. During this conversion time,
the LTC2499 will not respond to I2C/TWI requests. Thus, all calls interacting with the LTC2499 implement code to block until a sane timeout is
reached or the part starts responding. Be aware that when you use these functions, your code may hang up for a conversion time.

To minimize the wasted conversions, functions that read the current value and immediately change the channel (without wasting an entire
conversion cycle in between) are provided in addition to the standard read and channel change functions.

### Source Releases

The latest source code is always available from our GitHub [arduino-ard2499 repository](https://github.com/IowaScaledEngineering/arduino-ard2499).  We've broken it out
from the [main hardware project](https://github.com/IowaScaledEngineering/ard-ltc2499) to allow it to be included into Arduino's Library Manager automatically.


## Open Hardware & Software

Like all public Iowa Scaled Engineering designs, the ARD-LTC2499 is both open hardware and open source software.  The hardware is licensed under Creative Commons BY-SA, and the software is GPL v3.

The board was designed using the gEDA suite of tools (gschem, pcb, etc.), and the files are in the sch, sym, and pcb subdirectories.

