#!/usr/bin/env python

##     This file is part of the package "parapin".

##     The parapin package is free software; you can redistribute it
##     and/or modify it under the terms of the GNU Library General Public
##     License (LGPL) as published by the Free Software Foundation.

##     The parapin package is distributed in the hope that it will be
##     useful, but WITHOUT ANY WARRANTY; without even the implied
##     warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
##     See the GNU Library General Public License for more details.

##     You should have received a copy of the GNU Library General Public
##     License along with parapin; if not, write to the Free
##     Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
##     02111-1307 USA


##     For futher information on the parapin package, please refer to the
##     project information hosted on Sourceforge --

##     http://sourceforge.net/projects/parapin/

## 
 
## 
##    outputtest.py
##  
##    $Id $
##  
##


import os

# import parapin module
import parapin


# In parapin module you have access to all parapin functions
# (parapin.pin_init_user, pin_input_mode, pin_output_mode, pin_mode,
# set_pin, clear_pin, change_pin and pin_is_set) and a pulse_pin
# function described later. These functions are just a thin wrapper
# around parapin basic functions and should be used exactly like
# them. All constants are available in the parapin.CONST module, so
# you can use the line below to import all constants:

from parapin.CONST import *

# The module have two types: Port and Pin. Port objects work as a
# factory for Pin objects. You should not call parapin.Pin directly.

# Port type signature is:
# Port(base, uid=-1, inmode=-1, outmode=-1)

# The base argument is the port base I/O address used, exactly the
# same as the lp_base argument on the C version of Parapin. You may
# use the constants from the parapin.CONST module.

# Programs using Parapin must be running as root when the port is
# initialized, so in the Python they must be running as root when the
# parapin.Port object is created (otherwise, parapin.ParapinError
# exception will be raised). If you provide the uid argument, root
# privileges will be dropped to that user id after initialization.

# inmode and outmode arguments may be provided for convenience, so you
# don't have to call port.set_output_mode or port.set_input_mode later.

# Pin type signature is Pin(i), the i argument is the pin value to be
# used. The objects have access to all parapin functions (Pin.set,
# Pin.clear, Pin.change, Pin.is_set), the pulse function, lshift and
# rshift functions (described later) and support binary bitwise
# operators like & (and), | (or) and ^ (xor).

# So... first, create a Port object.
port = parapin.Port(LPT1, outmode=LP_DATA_PINS)

# You may also set input and output mode using port.set_output_mode()
# and port.set_input_mode() methods

# You have four options to Pin objects creation.

# 1. Calling the Pin class (not recommended):
pin_2 = parapin.Pin(LP_PIN02)
print "Seting pin 2"
pin_2.set()
raw_input("Hit return...\n")
print "Clearing pin 2"
pin_2.clear()
raw_input("Hit return...\n")

# 2. Calling the Port.raw_pin method (needs the value):
pin_3 = port.raw_pin(LP_PIN03)
print "Seting pin 3"
pin_3.set()
raw_input("Hit return...\n")
print "Clearing pin 3"
pin_3.clear()
raw_input("Hit return...\n")

# 3. Calling the Port.get_pin method (you may use just the index, like
# in the LP_PIN array)
# port.get_pin(i) -> port.raw_pin(LP_PIN[i])
pin_4 = port.get_pin(4)
print "Seting pin 4"
pin_4.set()
raw_input("Hit return...\n")
print "Clearing pin 4"
pin_4.clear()
raw_input("Hit return...\n")

# 4. Or you may use the port like a sequence.
# port[i] -> port.get_pin(i)
pin_5 = port[5]
print "Seting pin 5"
pin_5.set()
raw_input("Hit return...\n")
print "Clearing pin 5"
pin_5.clear()
raw_input("Hit return...\n")

# You may use an [i:j] slice to return a sequence of pins:
pin_6, pin_7, pin_8, pin_9 = port[6:10]

print "Seting pin 6"
pin_6.set()
raw_input("Hit return...\n")
print "Clearing pin 6"
pin_6.clear()

raw_input("Hit return...\n")
print "Seting pin 7"
pin_7.set()
raw_input("Hit return...\n")
print "Clearing pin 7"
pin_7.clear()
raw_input("Hit return...\n")

print "Seting pin 8"
pin_8.set()
raw_input("Hit return...\n")
print "Clearing pin 8"
pin_8.clear()
raw_input("Hit return...\n")

print "Seting pin 9"
pin_9.set()
raw_input("Hit return...\n")
print "Clearing pin 9"
pin_9.clear()
raw_input("Hit return...\n")

# The Port.get_pin function always return a single pin object, never a
# sequence, but it accepts any number of arguments.
# port.get_pin(x, y, z, ...) -> port.raw_pin(LP_PIN[x] | LP_PIN[y] | LP_PIN[x])

pins_2468 = port.get_pin(2, 4, 6, 8)

print "Setting pins 2, 4, 6 and 8"
pins_2468.set()
raw_input("Hit return...\n")

# Pin objects support bitwise operators...
pins_3579 = pin_3 | pin_5 | pin_7 | pin_9

print "Setting pins 3, 5, 7, 9"
pins_3579.set()
raw_input("Hit return...\n")

# And you may use the left and right shift operators (<< and >>), but
# with integers only!:

pin = port[2]
for i in xrange(2, 10):
    print "Clearing pin ", i
    pin.clear()
    pin = pin << 1
    raw_input("Hit return... \n")

# Pin objects have a pulse method, and may be used to send pulses
# Pin.pulse(pol, secs, msecs, n).

# The pol argument is the initial polarity of the pulse, if it should
# start setting or clearing the pin; secs and msecs are the time
# interval between each state change in seconds and milisseconds; n is
# the number of state changes (the number of complete pulses is n / 2).

# So, if you have pin 2 cleared, want to send 3 positive pulses
# (101010) with 10 ms between each state change you use:
# Pin.pulse(1, 0, 10, 6)

# If you want pin 2 to be set after that pulse train, (you want
# 1010101 this time) you just need to increment the state changes
# argument:
# Pin.pulse(1, 0, 10, 7)

print "Sending 3 positive 100ms pulses to each pin..."
raw_input("Hit return to start\n")
for i, pin in enumerate(port[2:10]):
    print "Pin ", i+2
    port[i].pulse(1, 0, 50, 6)
