DiscoLitez plugin, for controlling 32 LEDs or lamps via DeluxeUSB
or DeluxeUSBmux circuitry or compatible HID-USB devices.
See schematics and firmware in this .ZIP archive.

This plugin overcomes the absence of parallel ports,
especially on laptops.

All schematics and associated firmware (while very different
in construction) present the same USB-HID interface,
so the plugin needed is always the same.

Install this plugin by copying the file DLO_DeluxeUSB.dll
to the <WinAmp folder>\Plugins\DiscoLitez\DLOPs\ folder.
No other files are necessary.
If you do not find this folder, you probably should
install DiscoLitez first.

This plugin generally uses the first DeluxeUSB device found.

If you rename or copy the DLL to "DLO_DeluxeUSB 1.dll",
you can access a second DeluxeUSB device, and so forth.
Please note the size of the .DLL! Why bloating it?
(Hint to Delphi users: Don't use sysutils and VCL!)

This software is open-source freeware (public domain).
You can do with it what you want,
even including it into closed-source commercial programs.

Henrik Haftmann, Chemnitz, 2010/5/25

Change log:
+100407	First (preferred) schematic with ATtiny25 and four 74HC595, 32 single LEDs
+100408	First working sample
+100413	Second schematic with ATmega8, 48 or 88
*100419	Some updates (for beauty) on first schematic
*100424	Updated source file comments
+100523	Third schematic with ATtiny2313 and dot-matrix (7x5) LED display
-100613	DeluxeUSBmux2 schematic fixed

Todo:
Firmware for DeluxeUSBmux
