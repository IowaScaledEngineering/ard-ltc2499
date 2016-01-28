/*************************************************************************
Title:    ARD-LTC2499 Reflow Oven Monitor Sketch
Authors:  Michael Petersen <railfan@drgw.net>
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

#define LOG_INTERVAL 10
#define TEMPERATURE_ALARM 190.0

#define ALARM_PIN 7

#include <Wire.h>
#include <Ard2499.h>
#include <stdio.h>
#include <SD.h>
#include <SPI.h>

Ard2499 ard2499board1;

const int chipSelectSD = 4;
char fname[] = "00000000.log";
File logFile;

volatile uint8_t ticks;
volatile uint16_t decisecs=0;

uint8_t blinky = 0;
uint8_t logBlinky = 0;

void initialize100HzTimer(void)
{
	// Set up timer 1 for 100Hz interrupts
	TCCR1A = 0;
	TCCR1B = _BV(CS11) | _BV(CS10);
	TCCR1C = 0;
	TIMSK1 = _BV(TOIE1);
	ticks = 0;
	decisecs = 0;
}

ISR(TIMER1_OVF_vect)
{
	TCNT1 += 0xF63C;  // 0xFFFF - 0xF63C = 2500, 2500 * 64 * 100Hz = 16MHz
	if (++ticks >= 10)
	{
		ticks = 0;
		decisecs++;
	}
}

void setup()
{
  cli();//stop interrupts
  initialize100HzTimer();
  sei();
  
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);
  Wire.begin();
  ard2499board1.begin(ARD2499_ADC_ADDR_ZZZ, ARD2499_EEP_ADDR_ZZ);
  ard2499board1.ltc2499ChangeConfiguration(LTC2499_CONFIG2_60_50HZ_REJ);

  Serial.print((char)0x0C);  // Clear display
  Serial.print((char)0x1F);  // Set zoom to 2x
  Serial.print((char)0x28);
  Serial.print((char)0x67);
  Serial.print((char)0x40);
  Serial.print((char)0x02);
  Serial.print((char)0x02);

  // Configure IO pins
  pinMode(ALARM_PIN, OUTPUT);

  // Configure SD card chip select
  pinMode(chipSelectSD, OUTPUT);
  digitalWrite(chipSelectSD, HIGH);

  // see if the card is present and can be initialized:
  while(!SD.begin(chipSelectSD))
  {
    Serial.print((char)0x0B);  // Home position
    Serial.print(F("No SD Card"));
  }
  Serial.print((char)0x0B);  // Home position
  Serial.print(F("SD Card + "));
  decisecs = 0;
  while(decisecs < 10);
  
  // Create log file name
  while(SD.exists(fname))
  {
    uint8_t i = 7;
    fname[i]++;
    while(i > 0)   // CAUTION: not 100% safe.  The 1st character will continue to increment beyond the ASCII numbers
    {
      if(fname[i] > 0x39)
        {
          fname[i] = 0x30;
          fname[i-1]++;
        }
        i--;
    }
  }

  if(!(logFile = SD.open(fname, FILE_WRITE)))
  {
    Serial.print(F("Open Error"));
    while(1);
  }
  Serial.print((char)0x0B);  // Home position
  fname[8] = '\0';
  Serial.print(' ');
  Serial.print(fname);
  Serial.print(" ");
  fname[8] = '.';
  decisecs = 0;
  while(decisecs < 30);
}

float T;

void loop() {
  byte retval = 0;
  long adcVal;
  char string[16];
  float Vadc, Vcj, Tcj;
  
  ard2499board1.ltc2499ChangeChannel(LTC2499_CHAN_DIFF_1P_0N);

  adcVal = ard2499board1.ltc2499Read();
  Vadc = 1000 * ((adcVal*2.048) / 16777216.0);

//  Serial.print("Vadc: ");
//  Serial.print(Vadc, 6);
//  Serial.print("mV   ");

/*
  filter = ((adcVal-filter)/10) + filter;
  Serial.print(" : ");
  Serial.print(((filter*2.048) / 16777216.0), 6);
*/

  Tcj = ard2499board1.ltc2499ReadTemperature(ARD2499_TEMP_C);
  Vcj = computeColdJunctionVoltageTypeK(Tcj);
//  Serial.print("Tcj: ");
//  Serial.print(Tcj);
//  Serial.print("C   ");
//  Serial.print("Vcj: ");
//  Serial.print(Vcj);
//  Serial.print("mV   ");
  
  T = computeTemperatureTypeK(Vadc + Vcj);
//  Serial.print("Temperature: ");
//  Serial.print(T);
//  Serial.print("C\n");

//  char units = 'F';
//  T = (T * 1.8) + 32;  // Convert to Fahrenheit
  char units = 'C';

  if(T > TEMPERATURE_ALARM)
  {
    blinky ^= 1;
    digitalWrite(ALARM_PIN, HIGH);
  }
  else
  {
    blinky = 0;
    digitalWrite(ALARM_PIN, LOW);
  }
  
  // Print to VFD display
  Serial.print((char)0x0B);  // Home position
  Serial.print((char)31);
  Serial.print('L');
  Serial.print((char)16);
  Serial.print((char)0);
  Serial.print((char)0);
  Serial.print((char)(blinky*255));
  if(T <= -10)
    Serial.print(" ");
  else if(T < 0)
    Serial.print("  ");
  else if(T < 10)
    Serial.print("   ");
  else if(T < 100)
    Serial.print("  ");
  else
    Serial.print(" ");
  Serial.print(T, 1);
  Serial.print((char)248);
  Serial.print(units);
  Serial.print(" ");
  if(logBlinky)
    Serial.print(' ');
  else
    Serial.print((char)0xB2);
  
  if(decisecs > LOG_INTERVAL)
  {
    decisecs = 0;
    logBlinky ^= 1;
    logFile.println(T, 3);
    logFile.flush();
  }
}

/*
  Tables from:
  http://www.mosaic-industries.com/embedded-systems/microcontroller-projects/temperature-measurement/thermocouple/calibration-table
*/


float computeTemperatureTypeK(float V)
{
  // Type K equations
  // V = voltage in mV
  // Returns computed temperature in Celsius
  
  float T0, V0, p1, p2, p3, p4, q1, q2, q3;
  
  if( (-6.404 < V) && (V <= -3.554) )
  {
    T0 = -1.2147164E+02;
    V0 = -4.1790858E+00;
    p1 = 3.6069513E+01;
    p2 = 3.0722076E+01;
    p3 = 7.7913860E+00;
    p4 = 5.2593991E-01;
    q1 = 9.3939547E-01;
    q2 = 2.7791285E-01;
    q3 = 2.5163349E-02;
  }
  else if( (-3.554 < V) && (V <= 4.096) )
  {
    T0 = -8.7935962E+00;
    V0 = -3.4489914E-01;
    p1 = 2.5678719E+01;
    p2 = -4.9887904E-01;
    p3 = -4.4705222E-01;
    p4 = -4.4869203E-02;
    q1 = 2.3893439E-04;
    q2 = -2.0397750E-02;
    q3 = -1.8424107E-03;
  }
  else if( (4.096 < V) && (V <= 16.397) )
  {
    T0 = 3.1018976E+02;
    V0 = 1.2631386E+01;
    p1 = 2.4061949E+01;
    p2 = 4.0158622E+00;
    p3 = 2.6853917E-01;
    p4 = -9.7188544E-03;
    q1 = 1.6995872E-01;
    q2 = 1.1413069E-02;
    q3 = -3.9275155E-04;
  }
  else if( (16.397 < V) && (V <= 33.275) )
  {
    T0 = 6.0572562E+02;
    V0 = 2.5148718E+01;
    p1 = 2.3539401E+01;
    p2 = 4.6547228E-02;
    p3 = 1.3444400E-02;
    p4 = 5.9236853E-04;
    q1 = 8.3445513E-04;
    q2 = 4.6121445E-04;
    q3 = 2.5488122E-05;
  }
  else if( (33.275 < V) && (V <= 69.553) )
  {
    T0 = 1.0184705E+03;
    V0 = 4.1993851E+01;
    p1 = 2.5783239E+01;
    p2 = -1.8363403E+00;
    p3 = 5.6176662E-02;
    p4 = 1.8532400E-04;
    q1 = -7.4803355E-02;
    q2 = 2.3841860E-03;
    q3 = 0.0000000E+00;
  }
  else
  {
    return -1000;
  }

  float numer = (V - V0)*(p1 + (V - V0)*(p2 + (V - V0)*(p3 + p4*(V-V0))));
  float denom = 1 + (V - V0)*(q1 + (V - V0)*(q2 + q3*(V - V0)));
  return (T0 + (numer / denom));
}


float computeColdJunctionVoltageTypeK(float Tcj)
{
  // Type K equations
  // Tcj = temperature in Celsius
  // Returns equivalent voltage in mV
  float T0 = 25.0;
  float V0 = 1.0003453;
  float p1 = 4.0514854E-02;
  float p2 = -3.8789638E-05;
  float p3 = -2.8608478E-06;
  float p4 = -9.5367041E-10;
  float q1 = -1.3948675E-03;
  float q2 = -6.7976627E-05;
  
  float numer = (Tcj - T0)*(p1 + (Tcj - T0)*(p2 + (Tcj - T0) * (p3 + p4*(Tcj - T0))));
  float denom = 1 + (Tcj - T0) * (q1 + q2*(Tcj - T0));
  return (V0 + (numer / denom));
}
