#*************************************************************************
#Title:    
#Authors:  Michael Petersen
#          
#File:     
#
#*************************************************************************

BASE_NAME = ard2499_test2

BOARD  = arduino:avr:uno

ARDUINO = arduino-cli

help:
	@echo "make verify..... Build $(BASE_NAME)"
	@echo "make upload..... Flash the firmware"
	@echo "make terminal... Open uart terminal"

verify:
	$(ARDUINO) --verify --board $(BOARD) $(BASE_NAME).ino

upload:
	$(ARDUINO) compile -v -b $(BOARD) $(BASE_NAME).ino
	$(ARDUINO) upload -v -b $(BOARD) -p /dev/ttyUSB2 $(BASE_NAME).ino

terminal:
	miniterm.py --filter=direct /dev/ttyUSB2 115200
