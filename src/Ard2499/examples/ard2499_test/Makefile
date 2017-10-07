#*************************************************************************
#Title:    
#Authors:  Michael Petersen
#          
#File:     
#
#*************************************************************************

BASE_NAME = ard2499_test

#BOARD  = arduino:avr:uno
#PORT   = /dev/ttyUSB1
BOARD  = arduino:sam:arduino_due_x_dbg
PORT   = /dev/ttyACM0

ARDUINO = ~/arduino

help:
	@echo "make verify..... Build $(BASE_NAME)"
	@echo "make upload..... Flash the firmware"
	@echo "make terminal... Open uart terminal"

verify:
	$(ARDUINO) --verify --board $(BOARD) $(BASE_NAME).ino

upload:
	$(ARDUINO) --verbose --upload --board $(BOARD) $(BASE_NAME).ino

terminal:
	miniterm.py $(PORT) 9600
