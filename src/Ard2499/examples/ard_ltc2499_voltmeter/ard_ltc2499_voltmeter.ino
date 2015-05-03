/*************************************************************************
Title:    ARD-LTC2499 Volt Meter Example Sketch
Authors:  Michael Petersen <railfan@drgw.net>
File:     $Id: $
License:  GNU General Public License v3

LICENSE:
    Copyright (C) 2015 Nathan D. Holmes & Michael D. Petersen

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
#include <stdio.h>

Ard2499 ard2499board1;

void setup() {
	// initialize serial communications at 9600 bps:
	Serial.begin(9600);
	Wire.begin();
	ard2499board1.begin(ARD2499_ADC_ADDR_ZZZ, ARD2499_EEP_ADDR_ZZ);
	ard2499board1.ltc2499ChangeConfiguration(LTC2499_CONFIG2_60_50HZ_REJ);
	ard2499board1.ltc2499ChangeChannel(LTC2499_CHAN_DIFF_1P_0N);

	Serial.write(0x0C);  // Clear the display
	Serial.write(0x1F);  // Set 2x character magnification
	Serial.write(0x28);
	Serial.write(0x67);
	Serial.write(0x40);
	Serial.write(0x02);
	Serial.write(0x02);
}

float filter = 0;
bool first = true;

void loop() {
	long adcVal;
	float Vadc;

	adcVal = ard2499board1.ltc2499Read();
	Vadc = (adcVal*2.048) / 16777216.0;

	if(first)
	{
		filter = Vadc;  // Eliminate initial settling time
		first = false;
	}
	else
	{
		filter += (Vadc - filter) * 0.1;
	}

	Serial.write(0x0B);
	if(Vadc < 0)
		Serial.print("-");
	else
		Serial.print(" ");
	Serial.print(abs(filter), 6);
	Serial.print("V");
}

