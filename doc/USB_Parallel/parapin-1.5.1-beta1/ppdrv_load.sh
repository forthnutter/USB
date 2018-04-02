#!/bin/sh

# Loading script for parapindriver, should be run late in the boot process
# as root.  Kicking it off from the end of /etc/rc.local is usually fine.
# You must have parport.o, kparapin.o and parapindriver.o in a location
# that insmod can find them.

# The device name we want to use.  This can be whatever you choose.
DEVNAME="ppdrv_device"

# Be sure a depmod has been run, so the symbols in kparapin and
# parapindriver can be seen
depmod

# Make sure the requisite modules are loaded first.  Uncomment either
# of these that are not already being loaded in your configuration.
#modprobe parport
modprobe kparapin

# Load parapindriver, passing in the device name
modprobe parapindriver devname=${DEVNAME}

# Make our device node, using the dynamic major that was assigned to
# parapin driver when it registered itself.
rm -f /dev/${DEVNAME}
mknod /dev/${DEVNAME} u `grep $DEVNAME /proc/devices | cut -d " " -f 1` 0

# Change mode and ownership here if necessary
chmod 777 /dev/${DEVNAME}
chown root /dev/${DEVNAME}
chgrp root /dev/${DEVNAME}
