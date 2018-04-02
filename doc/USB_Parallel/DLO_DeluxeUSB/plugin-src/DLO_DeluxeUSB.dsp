# Microsoft Developer Studio Project File - Name="DLO_DeluxeUSB" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DLO_DeluxeUSB - Win32 Debug
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "DLO_DeluxeUSB.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "DLO_DeluxeUSB.mak" CFG="DLO_DeluxeUSB - Win32 Debug"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "DLO_DeluxeUSB - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DLO_DeluxeUSB - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DLO_DeluxeUSB - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD CPP /nologo /Gr /MT /W3 /O1 /I "c:\Programme\msvc\Platform SDK XP SP2\Include" /I "c:\Programme\msvc\ddk\inc\ddk\wdm\wxp" /D "WIN32" /FD /GF /c
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 kernel32.lib user32.lib setupapi.lib winmm.lib shlwapi.lib hid.lib /nologo /dll /pdb:none /machine:I386 /nodefaultlib /libpath:"c:\Programme\msvc\Platform SDK XP SP2\Lib" /libpath:"c:\Programme\msvc\ddk\lib\w2k\i386" /opt:nowin98 /release /largeaddressaware /merge:.rdata=.text

!ELSEIF  "$(CFG)" == "DLO_DeluxeUSB - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD CPP /nologo /Gr /MTd /W3 /Gm /ZI /Od /I "c:\Programme\msvc\Platform SDK XP SP2\Include" /I "c:\Programme\msvc\ddk\inc\ddk\wdm\wxp" /D "WIN32" /D "_DEBUG" /FR /FD /GF /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x407
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 kernel32.lib user32.lib setupapi.lib winmm.lib shlwapi.lib hid.lib /nologo /dll /debug /machine:I386 /nodefaultlib /out:"D:\Falco\Player\Plugins\DiscoLitez\DLOPs\DLO_DeluxeUSB.dll" /pdbtype:sept /libpath:"c:\Programme\msvc\Platform SDK XP SP2\Lib" /libpath:"c:\Programme\msvc\ddk\lib\w2k\i386" /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "DLO_DeluxeUSB - Win32 Release"
# Name "DLO_DeluxeUSB - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DLO_DeluxeUSB.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\DLO_DeluxeUSB.rc
# End Source File
# End Group
# End Target
# End Project
