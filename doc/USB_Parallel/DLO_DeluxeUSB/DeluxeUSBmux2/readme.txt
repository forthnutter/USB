[de]

Dieses Verzeichnis enthält Schaltplan, Leiterplattenentwurf und
Firmware für die „DeluxeUSBmux2“-Ausgabeeinheit.

Die Anschlussbelegung des ATtiny2313 ist auf einfache Leiterplatten-Topologie
für den Anschluss einer von zwei möglichen LED-Punktmatrixanzeigen optimiert.

Die Firmware ist kompatibel zum „DLO_DeluxeUSB.dll“-DiscoLitez-plugin.

Zum Editieren der Schaltung/Leiterplatte „Eagle“  von www.cadsoft.de benutzen.
Die gemeinsame Eagle-Bibliothek „dl.lbr“ liegt in ../DeluxeUSB/schematic/.

Zum Kompilieren und Brennen der Firmware „WinAVR“ (mit gcc) benutzen.
Die gemeinsam genutzte V-USB-Bibliothek liegt in ../DeluxeUSB/firmware/usbdrv/.

Dim Gegensatz zu DeluxeUSBmux genügt hier ein einfacherer Mikrocontroller
für bis zu 40 (5x8) Leuchtdioden.
Für Glühlampen oder gar TRIACs ist diese Schaltung nicht sonderlich geeignet!

[en]

This directory contains the schematic, a possible board and the firmware
for the “DeluxeUSBmux2” output device.

The pinout of the ATtiny2313 firmware is optimized for ease making a PCB
that controls one of two possible standard dot matrix LED fields.

The firmware is made compatible to DLO_DeluxeUSB.dll DiscoLitez plugin.

To edit the schematic and/or board, use “Eagle” software from www.cadsoft.de.
Use the common library file “dl.lbr” from ../DeluxeUSB/schematic/.

To compile and flash the firmware, use “WinAVR” package with gcc compiler.
Use the common V-USB files out of ../DeluxeUSB/firmware/usbdrv/.

The multiplexed LED matrix is /not/ very suitable for driving TRIACs!


haftmann#software, 100525
