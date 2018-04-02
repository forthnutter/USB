DiscoLitez plugin, for controlling eight channels via
 a regular USB->Printer adapter

This plugin overcomes the absence of parallel ports,
especially on laptops.

Requires following connections, besides such an adapter:
- Connection of /STB (1) and /ACK (10) for handshake
- Connection of BSY (11) with GND (22)
- Maybe a connection of PE (12) with GND (23)
- Maybe a connection of /ERR (15) with /INI (16) supplying HIGH level
- Catching data with a 8-channel flip-flop (e.g. 74HC574),
  CLOCK = STB (1), Ucc = /INI (16), GND = /OE = GND (21)
Some light controllers may have these connections built-in,
for emulating a regular printer.

See schematics.

Install this plugin by copying the file DLO_UsbPrn.dll
to the <WinAmp folder>\Plugins\DiscoLitez\DLOPs\ folder.
No other files are necessary.
If you do not find this folder, you probably should
install DiscoLitez first.

This plugin uses the first USB->Printer adapter found by default.
Copy this DLL to "DLO_UsbPrn#1.dll" etc. to get a driver
for the second, third etc. USB->Printer adapter.

Please note the size of the .DLL! Why bloating it?

This software is open-source freeware (public domain).
You can do with it what you want,
even including it into closed-source commercial programs.

Henrik Haftmann, Chemnitz, 2009/2/12

Change log:
*080817	Added /GF compiler switch reduces DLL size by 512 bytes
-080820	Wrong output lights, timer procedure
+110403	User-selectable number or USB->Printer adapter
-110404	Asynchronous operation for reliability when adapter is unplugged
