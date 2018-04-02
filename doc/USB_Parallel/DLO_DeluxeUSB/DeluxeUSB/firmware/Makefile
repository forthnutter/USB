PROJECT = DeluxeUSB
DEVICE  = attiny25
F_CPU   = 16500000 # in Hz
AVRDUDE = avrdude -c pony-stk200 -P lpt1 -p $(DEVICE)

CFLAGS  = -Iusbdrv -I.
OBJECTS = $(PROJECT).o usbdrv/usbdrvasm.o
#usbdrv/usbdrv.o 

COMPILE = avr-gcc -Wall -Os -DF_CPU=$(F_CPU) $(CFLAGS) -mmcu=$(DEVICE)

############################## ATTiny25/45/85 ###############################
# ATtiny*5 FUSE_L (Fuse low byte):
# 0xC1 = 1 1 0 0   0 0 0 1
#        ^ ^ \+/   \--+--/
#        | |  |       +------- CKSEL 3..0 (clock selection -> HF PLL @ 16.5 MHz)
#        | |  +--------------- SUT 1..0 (BOD enabled, fast rising power)
#        | +------------------ CKOUT (clock output on CKOUT pin -> disabled)
#        +-------------------- CKDIV8 (divide clock by 8 -> don't divide)
# ATtiny*5 FUSE_H (Fuse high byte):
# 0xD5 = 1 1 0 1   0 1 0 1
#        ^ ^ ^ ^   ^ \-+-/ 
#        | | | |   |   +------ BODLEVEL 2..0 (brownout trigger level -> 2.7V)
#        | | | |   +---------- EESAVE (preserve EEPROM on Chip Erase -> preserve)
#        | | | +-------------- WDTON (watchdog timer always on -> no)
#        | | +---------------- SPIEN (enable serial programming -> enabled)
#        | +------------------ DWEN (debug wire enable)
#        +-------------------- RSTDISBL (disable external reset -> enabled)
#
LFUSE = 0xC1
HFUSE = 0xD5

all: $(PROJECT).hex

help:
	@echo "make all ....... to build DeluxeUSB.hex out of source files"
	@echo "make prog ...... to flash fuses and firmware (for ATtiny25)"
	@echo "make tn45 ...... to flash fuses and firmware for ATtiny45"
	@echo "make tn85 ...... to flash fuses and firmware for ATtiny85"
	@echo "make fuse ...... to flash the fuses"
	@echo "make flash ..... to flash the firmware"
	@echo "make clean ..... to delete objects and hex file"
	@echo "make disasm .... create DeluxeUSB.dis"

# rule for programming fuse bits only:
fuse:
	$(AVRDUDE) -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m

# rule for uploading firmware, compile if necessary:
flash: $(PROJECT).hex
	$(AVRDUDE) -U flash:w:$<:i

# program a fresh chip without compiling, only one invocation of avrdude
prog:
	$(AVRDUDE) -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m -U flash:w:$(PROJECT).hex:i

# program an ATtiny45 chip without compiling	
tn45:	
	$(AVRDUDE) -p attiny45 -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m -U flash:w:$(PROJECT).hex:i

# program an ATtiny85 chip without compiling	
tn85:	
	$(AVRDUDE) -p attiny85 -U hfuse:w:$(HFUSE):m -U lfuse:w:$(LFUSE):m -U flash:w:$(PROJECT).hex:i

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
