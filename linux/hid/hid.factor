! Copyright (C) 2011 Joseph Louis Moschini.
! See http://factorcode.org/license.txt for BSD license.

USING: kernel alien alien.c-types alien.syntax alien.data alien.strings
       namespaces locals literals math math.parser prettyprint
       arrays classes.struct memory accessors semantic-versioning
       sequences byte-arrays tools.continuations libudev system-info.linux ;

IN: usb.hid.linux

TUPLE: hid-device handle blocking reports ;

! hidapi info structure
TUPLE: hid_device_info path vendor_id product_id serial_number
    release_number manufacturer_string product_string usage_page
    usage interface_number *next ;

! #define LINUX_VERSION_CODE 200711
! #define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
SYMBOL: kernel_version ;


: detect-kernel-version ( -- kv )
    release split-version first
    first3 [ 16 shift ] 2dip [ 8 shift ] dip + + ;


! new hid device
: <hid-device> ( -- hid-device )
    hid_device new
    f >>handle
    t >>blocking
    0 >>reports ;



: hid_init ( -- )
    detect_kernel_version kernel_ersion set
	return 0;
}
