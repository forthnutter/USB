! Copyright (C) 2011 Joseph Louis Moschini.
! See http://factorcode.org/license.txt for BSD license.

USING: kernel alien alien.c-types alien.syntax alien.data alien.strings
       namespaces locals literals math math.parser prettyprint
       windows windows.kernel32 windows.ddk.setupapi windows.ddk.hid
       windows.ole32 windows.errors arrays classes.struct memory accessors
       sequences byte-arrays windows.types tools.continuations ;

IN: usb.hid.linux

TUPLE: hid-device handle blocking reports ;


: detect-kernel-version ( -- kv )
    uname ;


