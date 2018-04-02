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
##    test_parapin.py
##  
##    $Id $
##  
##



import unittest
import gc
gc.set_debug(gc.DEBUG_LEAK)


#import time

import parapin, os, time
from parapin.CONST import *

class TestPin(unittest.TestCase):
    def setUp(self):
        self.port = parapin.Port(0x378)
        self.port.set_output_mode(LP_DATA_PINS)
        
    def test_pin_attributes(self):
        pin = parapin.Pin(0)
        self.assertEqual(pin.pin, 0)
        self.assert_(callable(pin.set))
        self.assert_(callable(pin.clear))
        self.assert_(callable(pin.change))
        self.assert_(callable(pin.is_set))
        self.assert_(callable(pin.pulse))
        self.assert_(callable(pin.__nonzero__))
        self.assert_(callable(pin.__and__))
        self.assert_(callable(pin.__lshift__))
        self.assert_(callable(pin.__rshift__))

    def test_pin_methods(self):
        pin = self.port.get_pin(9)
        self.assertEqual(pin.pin, LP_PIN09)
        pin.set()
        pin.clear()
        pin.change(1)
        pin.change(0)
        pin.pulse(1, 0, 10, 10)
        self.assert_(bool(pin.is_set()) is bool(pin))
        self.assert_(pin.is_set() == pin.__nonzero__())

    def test_pin_and(self):
        pin1 = self.port.get_pin(1)
        pin2 = self.port.get_pin(2)
        pin1and2 = pin1 & pin2
        self.assertEqual(pin1and2.pin, pin1.pin & pin2.pin)
        
    def test_pin_or(self):
        pin1 = self.port[1]
        pin2 = self.port[2]
        pin1or2 = pin1 | pin2
        self.assertEqual(pin1or2.pin, pin1.pin | pin2.pin)
        
    def test_pin_xor(self):
        pin1 = self.port[1]
        pin2 = self.port[2]
        pin1xor2 = pin1 ^ pin2
        self.assertEqual(pin1xor2.pin, pin1.pin ^ pin2.pin)

    def test_pin_lshift(self):
        pin2 = self.port.get_pin(2)
        pin5 = pin2 << 3 
        self.assertEqual(pin5.pin, LP_PIN05)
        pin9 = pin5 << 4
        self.assertEqual(pin9.pin, LP_PIN09)
        self.assertRaises(ValueError, self.do_lshift, pin2, pin5)

    def test_pin_rshift(self):
        pin9 = self.port.get_pin(9)
        pin4 = pin9 >> 5
        self.assertEqual(pin4.pin, LP_PIN04)
        pin2 = pin4 >> 2
        self.assertEqual(pin2.pin, LP_PIN02)
        self.assertRaises(ValueError, self.do_rshift, pin9, pin2)

    def do_lshift(self, a, b):
        return a << b

    def do_rshift(self, a, b):
        return a >> b

    def test_coerce(self):
        pin1 = self.port[1]
        pin2 = self.port[2]
        
        _pin1, _pin2 = coerce(pin1, pin2)
        self.assert_(_pin1 is pin1)
        self.assert_(_pin2 is pin2)

        self.assertRaises(TypeError, coerce, pin1, "")
        

class TestPort(unittest.TestCase):
    def setUp(self):
        self.port = parapin.Port(0x378)
        self.port.set_output_mode(LP_DATA_PINS)

    def raise_parapin_error(self):
        raise parapin.ParapinError
    
    def test_exception(self):
        self.assertRaises(parapin.ParapinError, self.raise_parapin_error)
    
    def test_init_with_no_arguments(self):
        self.assertRaises(TypeError, parapin.Port)

    def test_init_with_arguments(self):
        port = parapin.Port(0x378)
        self.assertEqual(port.base, 0x378)
        self.assertEqual(os.getuid(), 0)

    def test_get_single_pin(self):
        for i in xrange(1, 26):
            pin = self.port.get_pin(i)
            self.assertEqual(pin.pin, LP_PIN[i])
            if i < 18:
                self.assertEqual(pin.pin, eval("LP_PIN%02i"%i))

    def test_range_error(self):
        self.assertRaises(IndexError, self.port.get_pin, 26)
        self.assertRaises(IndexError, self.port.get_pin, -1)
        self.assertRaises(IndexError, self.port.get_pin, 1, 2, 26)
        self.assertRaises(IndexError, self.port.get_pin, -1, 1, 15)
        self.assertRaises(IndexError, self.port.get_pin, -1, 5, 26)
        self.assertRaises(IndexError, self.port.__getitem__, 26)
        self.assertRaises(IndexError, self.port.__getitem__, -1)
        
    def test_get_multiple_pins(self):
        pins23 = self.port.get_pin(2, 3)
        self.assertEqual(pins23.pin, LP_PIN02 | LP_PIN03)
        pins234 = self.port.get_pin(2, 3, 4)
        self.assertEqual(pins234.pin, LP_PIN02 | LP_PIN03 | LP_PIN04)

    def test_port_get_item(self):
        for i in xrange(1, 26):
            pin = self.port[i]
            self.assertEqual(pin.pin, LP_PIN[i])
            if i < 18:
                self.assertEqual(pin.pin, eval("LP_PIN%02i"%i))

    def test_port_get_tuple(self):
        pin5, pin6, pin7 = self.port[5:8]
        self.assertEqual(pin5.pin, LP_PIN05)
        self.assertEqual(pin6.pin, LP_PIN06)
        self.assertEqual(pin7.pin, LP_PIN07)        


class TestPortWithOutputMode(TestPort):
    def setUp(self):
        self.port = parapin.Port(0x378, outmode=LP_DATA_PINS)

if __name__ == '__main__':
    try:
        unittest.main()
    finally:
        port = parapin.Port(0x378, 1000)
        assert os.getuid() == 1000
        
        print gc.garbage
