PROJECT = DeluxeUSBmux2
DEVICE  = attiny2313
F_CPU   = 12000000 # in Hz
AVRDUDE = avrdude -c pony-stk200 -P lpt1 -p $(DEVICE)

CFLAGS  = -Iusbdrv -I.
OBJECTS = $(PROJECT).o usbdrv/usbdrvasm.o
#usbdrv/usbdrv.o 

COMPILE = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)

############################## ATtiny2313 ###############################
# ATtiny2313 FUSE_L (Fuse low byte):
# 0xEE = 1 1 1 0   1 1 1 0
#        ^ ^ \+/   \--+--/
#        | |  |       +------- CKSEL 3..0 (clock selection -> crystal)
#        | |  +--------------- SUT 1..0 (fastest startup possible)
#        | +------------------ CKOUT (clock output on CKOUT pin -> disabled)
#        +-------------------- CKDIV8 (divide clock by 8 -> don't divide)
# ATtiny2313 FUSE_H (Fuse high byte):
# 0xDB = 1 1 0 1   1 0 1 1
#        ^ ^ ^ ^   \-+-/ ^
#        | | | |     |   +---- RSTDISBL (disable external reset -> enabled)
#        | | | |     +-------- BODLEVEL 2..0 (brownout trigger level -> 2.7V)
#        | | | +-------------- WDTON (watchdog timer always on -> no)
#        | | +---------------- SPIEN (enable serial programming -> enabled)
#        | +------------------ EESAVE (preserve EEPROM on Chip Erase -> preserve)
#        +-------------------- DWEN (debug wire enable)
#
LFUSE = 0xEE
HFUSE = 0xDB
EFUSE = 0xFF

all: $(PROJECT).hex

help:
	@echo "make all ....... to build DeluxeUSBmux2.hex out of source files"
	@echo "make prog ...... to flash fuses and firmware"
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware"
	@echo "make clean ..... to delete objects and hex file"
	@echo "make disasm .... create DeluxeUSBmux2.dis"

# rule for programming fuse bits only:
fuse:
	$(AVRDUDE) -U efuse:w:$(EFUSE):m -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

# rule for uploading firmware, compile if necessary:
flash: $(PROJECT).hex
	$(AVRDUDE) -U flash:w:$<:i

# program a fresh chip without compiling, only one invocation of avrdude
prog:
	$(AVRDUDE) -U efuse:w:$(EFUSE):m -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m -U flash:w:$(PROJECT).hex:i

# rule for deleting dependent files (those which can be built by Make):
clean:
	rm -f $(PROJECT).hex $(PROJECT).lst $(PROJECT).elf *.o usbdrv/*.o

# Generic rule for compiling C files:
.c.o:
	$(COMPILE) -c $< -o $@

# Generic rule for assembling Assembler source files:
.S.o:
	$(COMPILE) -x assembler-with-cpp -c $< -o $@

# file targets:

$(PROJECT).elf: $(OBJECTS)
	$(COMPILE) -o $@ $(OBJECTS)

$(PROJECT).hex: $(PROJECT).elf
	rm -f $@
	avr-objcopy -j .text -j .data -O ihex $< $@
	avr-size $@

disasm:	$(PROJECT).elf
	avr-objdump -d $< > $(PROJECT).lst
