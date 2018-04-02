Another DiscoLitez plugin, replacing huge DLO_NTParallelXxx.dll plugins.

Install this plugin by copying the file DLO_Parallel 0x378.dll
to the <WinAmp folder>\Plugins\DiscoLitez\DLOPs\ folder.
No other files are necessary.
If you do not find this folder, you probably should
install DiscoLitez first.

The port address that is used is part of the DLL file name.
To change the port address, simply rename the DLL accordingly.
No recompilation is necessary.

A user can change the port address in configuration dialog,
this will rename the DLL too.

This DLL requires inpout32.dll beside or in PATH.
Note that this inpout32.dll is much smaller but has the same function
as the original one that floats (or bloats) in the Internet.
You can find its (slightly modified) source at
http://www-user.tu-chemnitz.de/~heha/hs_freeware/inpout32_source_and_bins_changed.zip

Change log:
-080820	Wrong output fixed, sections .text and .rdata combined
*091112	This new (and unfortunately larger) inpout32.dll runs on X64 platforms too;
	it contains two signed kernel-mode drivers and loads the appropriate
	Note that you don't need inpout64.dll for this plugin because this plugin
	is fixed to be 32 bit, because DiscoLitez is 32 bit, because WinAmp is 32 bit.
	IF (and only if) DiscoLitez comes out as 64 bit, please contact me for
	compiling 64 bit output plugins.

Henrik Haftmann, 090424
