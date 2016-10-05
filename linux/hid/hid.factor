! Copyright (C) 2011 Joseph Louis Moschini.
! See http://factorcode.org/license.txt for BSD license.

USING: kernel alien alien.c-types alien.syntax alien.data alien.strings
       namespaces locals literals math math.parser prettyprint
       arrays classes.struct memory accessors
       sequences byte-arrays tools.continuations libudev system-info.linux ;

IN: usb.hid.linux

TUPLE: hid-device handle blocking reports ;


: detect-kernel-version ( -- kv )
    version ;


