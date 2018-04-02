[de]

Dieses Verzeichnis enthält Schaltplan, Leiterplattenentwurf und
Firmware für die „DeluxeUSB“-Ausgabeeinheit.

Die Anschlussbelegung des ATtiny25 ist auf einfache Leiterplatten-Topologie
für den Einbau in einen USB-Stecker optimiert.

Die Firmware ist kompatibel zum „DLO_DeluxeUSB.dll“-DiscoLitez-Plugin.

Zum Editieren der Schaltung/Leiterplatte „Eagle“  von www.cadsoft.de benutzen.
Zum Kompilieren und Brennen der Firmware „WinAVR“ (mit gcc) benutzen.

Der USB-Pullup-Widerstand R1 sollte wirklich mit 4,7 kΩ bestückt werden!
Nicht 1,5 kΩ, nicht 10 kΩ.
Nur dieser Wert erfüllt die Bedingung Standby-Stromaufnahme < 500 µA.
Das Problem ist die Eingangsspannung bei etwa halber Betriebsspannung.

Das Konzept der Deluxe32-Schaltung mit den 4 Stück 74HCT?[35]7[34]
wurde zu Gunsten von 74HCT?595 verworfen, um mit dem kleinen 8-poligen
Mikrocontroller auszukommen.
Für die Potenzialtrennung zur Steuerung von TRIACs genügen 3 schnelle
(Opto-)Koppler statt 32 langsame. Billiger sind induktive Koppler.


[en]

This directory contains the schematic, a possible board and the firmware
for the “DeluxeUSB” output device.

The pinout of the ATtiny25 is optimized for making a PCB into the
standard USB plug housing replacing the original contact plates.

The firmware is made compatible to DLO_DeluxeUSB.dll DiscoLitez plugin.

To edit the schematic and/or board, use “Eagle” software from www.cadsoft.de.
To compile and flash the firmware, use “WinAVR” package with gcc compiler.

You should really populate the USB pullup resistor R1 with 4.7 kΩ!
Neither 1.5 kΩ, nor 10 kΩ. This value is required for standby power
consumption < 500 µA as required by USB specification.
The input pin voltage near half of supply voltage is the probem here.

This design favours 74HC595 over 74HC374, to use a small 8-pin controller.
Moreover, this concept needs only 3 fast couplers for driving mains-sided
TRIACs instead of 32 couplers in traditional designs.


haftmann#software, 100413
