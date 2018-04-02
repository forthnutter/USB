[de]

Dieses Verzeichnis enthält Schaltplan, Leiterplattenentwurf und
Firmware für die „DeluxeUSBmux“-Ausgabeeinheit.

Die Anschlussbelegung des ATmega48 ist auf einfache Leiterplatten-Topologie
für den Anschluss einer LED-Punktmatrixanzeige optimiert.

Die Firmware ist kompatibel zum „DLO_DeluxeUSB.dll“-DiscoLitez-plugin.

Zum Editieren der Schaltung/Leiterplatte „Eagle“  von www.cadsoft.de benutzen.
Die gemeinsame Eagle-Bibliothek „dl.lbr“ liegt in ../DeluxeUSB/schematic/.

Zum Kompilieren und Brennen der Firmware „WinAVR“ (mit gcc) benutzen.
Die gemeinsam genutzte V-USB-Bibliothek liegt in ../DeluxeUSB/firmware/usbdrv/.

Die gemultiplexte LED-Matrix ist zum Ansteuern von TRIACs /nicht/ sonderlich geeignet!
Dafür benutze man eher den SPI-Ausgang wie in DeluxeUSB,
den DeluxeUSB als solchen oder ein DMX-Ausgabegerät wie in
http://www-user.tu-chemnitz.de/~heha/Mikrocontroller/Triac8/

[en]

This directory contains the schematic, a possible board and the firmware
for the “DeluxeUSBmux” output device.

The pinout of the ATmega48 is optimized for making a PCB that controls
a standard dot matrix LED field.

The firmware is made compatible to DLO_DeluxeUSB.dll DiscoLitez plugin.

To edit the schematic and/or board, use “Eagle” software from www.cadsoft.de.
Use the common library file “dl.lbr” from ../DeluxeUSB/schematic/.

To compile and flash the firmware, use “WinAVR” package with gcc compiler.
Use the common V-USB files out of ../DeluxeUSB/firmware/usbdrv/.

The multiplexed LED matrix is /not/ very suitable for driving TRIACs!
For this purpose, use the SPI output as shown in DeluxeUSB,
use DeluxeUSB instead, or use a DMX device as shown at
http://www-user.tu-chemnitz.de/~heha/Mikrocontroller/Triac8/

haftmann#software, 100409
