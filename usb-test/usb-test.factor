



USING:  kernel alien alien.c-types alien.data libusb byte-arrays namespaces
        tokyo.utils tools.hexdump ;

IN: usb.usb-test

SYMBOLS: dev cnt ;


: usb-start ( -- )
    f libusb_init
    [
        f
        4 <byte-array> dev set
        dev get
        libusb_get_device_list cnt set
        dev get malloc-byte-array
        
        


    ] when

    f libusb_exit
    ;



    
