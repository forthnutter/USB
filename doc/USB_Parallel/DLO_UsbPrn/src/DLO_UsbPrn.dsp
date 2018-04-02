# Microsoft Developer Studio Project File - Name="DLO_UsbPrn" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=DLO_UsbPrn - Win32 Debug
!MESSAGE "DLO_UsbPrn - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE "DLO_UsbPrn - Win32 Debug" (basierend auf  "Win32 (x86) Dynamic-Link Library")

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "DLO_UsbPrn - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD CPP /nologo /Gr /MT /W3 /O1 /I "c:\Programme\msvc\Platform SDK XP SP2\Include" /I "c:\Programme\msvc\ddk\inc\ddk\wdm\wxp" /D "WIN32" /FD /GF /c
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD RSC /l 0x409
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD LINK32 kernel32.lib user32.lib shlwapi.lib setupapi.lib winmm.lib /nologo /dll /pdb:none /machine:I386 /nodefaultlib /libpath:"c:\Programme\msvc\Platform SDK XP SP2\Lib" /opt:nowin98 /release /largeaddressaware /merge:.rdata=.text

!ELSEIF  "$(CFG)" == "DLO_UsbPrn - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 1
# PROP Target_Dir ""
# ADD CPP /nologo /Gr /MTd /W3 /Gm /ZI /Od /I "c:\Programme\msvc\Platform SDK XP SP2\Include" /I "c:\Programme\msvc\ddk\inc\ddk\wdm\wxp" /D "WIN32" /D "_DEBUG" /FR /FD /GF /c
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD LINK32 kernel32.lib user32.lib shlwapi.lib setupapi.lib winmm.lib /nologo /dll /debug /machine:I386 /nodefaultlib /out:"c:\Programme\Winamp5\Plugins\DiscoLitez\DLOPs/DLO_UsbPrn.dll" /pdbtype:sept /libpath:"c:\Programme\msvc\Platform SDK XP SP2\Lib" /opt:nowin98
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "DLO_UsbPrn - Win32 Release"
# Name "DLO_UsbPrn - Win32 Debug"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\DLO_UsbPrn.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# End Group
# Begin Group "Ressourcendateien"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\DLO_UsbPrn.rc
# End Source File
# End Group
# End Target
# End Project
