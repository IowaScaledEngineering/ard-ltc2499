/*************************************************************************
Title:    ARD-LTC2499 Library Example Arduino Sketch
Authors:  Nathan D. Holmes <maverick@drgw.net>
File:     $Id: $
License:  GNU General Public License v3

LICENSE:
    Copyright (C) 2013 Nathan D. Holmes & Michael D. Petersen

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

Ard2499 ard2499board;

byte confChan=0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  Wire.begin();
  ard2499board.begin(ARD2499_ADC_ADDR_ZZZ, ARD2499_EEP_ADDR_ZZ);
  ard2499board.ltc2499ChangeConfiguration(LTC2499_CONFIG2_60_50HZ_REJ);

  ard2499board.ltc2499ChangeChannel(LTC2499_CHAN_DIFF_1P_0N);
}

int32_t filter01, filter23;

void loop() {
  int32_t adcVal;
  
  adcVal = ard2499board.ltc2499ReadAndChangeChannel(LTC2499_CHAN_DIFF_3P_2N);
  filter01 = ((adcVal + 15*filter01)+8) / 16;

  Serial.print("Ch01,");
  Serial.print(((adcVal*2.048) / 16777216.0), 6);
  Serial.print(",");
  Serial.print(((filter01*2.048) / 16777216.0), 6);
  Serial.print(",");

  adcVal = ard2499board.ltc2499ReadAndChangeChannel(LTC2499_CHAN_DIFF_1P_0N);
  filter23 = ((adcVal + 15*filter23)+8) / 16;

  Serial.print("Ch23,");
  Serial.print(((adcVal*2.048) / 16777216.0), 6);
  Serial.print(",");
  Serial.print(((filter23*2.048) / 16777216.0), 6);
  Serial.print(",");

  Serial.print("Total,");
  Serial.print((((filter01+filter23)*2.048) / 16777216.0), 6);

  Serial.println("");
}


