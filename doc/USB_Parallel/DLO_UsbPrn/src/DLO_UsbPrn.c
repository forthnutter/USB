/* Discolitez plugin, for controlling eight channels via
 * a regular usb printer adapter
 * Requires following connections, besides such an adapter:
 * - Connection of /STB (1) and /ACK (10) for handshake
 * - Connection of BSY (11) with GND (22)
 * - Connection of PE (12) with GND (23)
 * - Connection of /ERR (15) with /INI (16) supplying HIGH level
 * - Catching data with a 8-channel flip-flop (e.g. 74HC574),
 *   CLOCK = STB (1), Ucc = /INI (16), GND = /OE = GND (21)
 * Some light controllers may have these connections built-in,
 * for emulating a regular printer.
 *
 * This plugin uses the first USB-Printer adapter found by default.
 * It takes the number of the adapter from DLL file name,
 * e.g. "DLO_UsbPrn 1.dll" uses the second UsbPrn found.
 *
 * USB->Printer adapters may behave slightly differently!
 * no-name - Prolific PL-2305H: /SELECT=L, /AF=H, ONLINE not checked
 * LogiLink - WCH CH340S: /SELECT=toggling, /AF=toggling, ONLINE not checked
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <usb.h>
#include <usbiodef.h>
#include <usbprint.h>
#include <setupapi.h>
#include <devguid.h>
#include <wdmguid.h>
#include <winioctl.h>


/* This define is required so that the GUID_DEVINTERFACE_USBPRINT variable is
 * declared an initialised as a static locally, since windows does not include it in any
 * of its libraries
 */
static const GUID GUID_DEVINTERFACE_USBPRINT = {
  0x28d78fad,0x5a12,0x11D1,0xae,0x5b,0x00,0x00,0xf8,0x03,0xa8,0xc2};

static HANDLE OpenUsbPort(int n, DWORD flags) {
 HANDLE ret = 0;
 HDEVINFO devs = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USBPRINT,0,0,DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);
 if (devs != INVALID_HANDLE_VALUE) {
  SP_DEVICE_INTERFACE_DATA devinterface;
  devinterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
  if (SetupDiEnumDeviceInterfaces(devs, NULL, &GUID_DEVINTERFACE_USBPRINT, n, &devinterface)) {
   SP_DEVINFO_DATA devinfo;
   struct{
    SP_DEVICE_INTERFACE_DETAIL_DATA id;
    TCHAR space[MAX_PATH];
   }id;
   devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
   id.id.cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
   if (SetupDiGetDeviceInterfaceDetail(devs, &devinterface, &id.id, sizeof(id), NULL, &devinfo)) {
    ret = CreateFile(id.id.DevicePath,GENERIC_WRITE,FILE_SHARE_READ,
      NULL,OPEN_EXISTING,flags,NULL);
    if (ret == INVALID_HANDLE_VALUE) ret = 0;
   }
  }
  SetupDiDestroyDeviceInfoList(devs);
 }
 return ret;
}

static HANDLE usbHandle;
static BYTE PortState;
static int DevNum;	// zero-based device number
static UINT TimerId;
static HINSTANCE hInst;
static OVERLAPPED o;


// Retrieves the DLL file name into buffer and returns a pointer
// to the first ' ' or the ".dll" portion of the file name.
// Returns NULL when failing.
static PSTR GetNumberPointer(PSTR p, size_t cch) {
 PSTR q;
 if (!GetModuleFileName(hInst,p,cch)) return NULL;
 p=PathFindFileName(p);
 if (!p) return p;
 q=StrPBrk(p," #");
 if (!q) q=PathFindExtension(p);
 return q;
}

static void OutPortState(BYTE State) {
 DWORD bw;
 if (!usbHandle) return;
 if (!WriteFile(usbHandle,&State,1,&bw,&o)
 && (GetLastError()!=ERROR_IO_PENDING
 || WaitForSingleObject(o.hEvent,10))) {
#ifdef _DEBUG
  char s[128];
  bw=GetLastError();
  wnsprintf(s,sizeof(s),"USB-LPT error %d\n",bw);
  OutputDebugString(s);
#endif
  CancelIo(usbHandle);
  if (!DeviceIoControl(usbHandle,IOCTL_USBPRINT_SOFT_RESET,NULL,0,NULL,0,&bw,&o)
  && (GetLastError()!=ERROR_IO_PENDING
  || WaitForSingleObject(o.hEvent,10))) {
   CancelIo(usbHandle);
  }
 }
}

// Let a timer procedure actualize the output,
// reduce load due to too many kernel<->user mode transitions
static void CALLBACK TimerProc(UINT TimerId, UINT uMsg, DWORD dwUser,
  DWORD dw1, DWORD dw2) {
 static BYTE NewState;
 if (NewState!=PortState) OutPortState(NewState=PortState);
}

/**********************
 * exported functions *
 **********************/
BYTE _declspec(dllexport) _cdecl DLO_getnch(void) {
 return 8;
}

LPSTR _declspec(dllexport) _cdecl DLO_getname(void) {
 static char buf[32];
 wnsprintf(buf,sizeof(buf),"USB->Printer adapter #%d",DevNum);
 return buf;
}

void _declspec(dllexport) _cdecl DLO_datain(BYTE ch, BYTE val) {
 BYTE mask = 1<<(ch-1);	// The bit to be set or reset
 if (val) {		// Set bit?
  if (PortState&mask) return;	// Already set, do nothing
 }else{			// Reset bit?
  if (~PortState&mask) return;	// Already reset, do nothing
 }
 PortState ^= mask;	// Swap bit, the timer procedure will output this value
}

void _declspec(dllexport) _cdecl DLO_init(void) {
 if ((usbHandle=OpenUsbPort(DevNum,FILE_FLAG_OVERLAPPED))) {
  o.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
  timeBeginPeriod(10);	// Let MMSYSTEM create the necessary thread
  TimerId=timeSetEvent(10,10,TimerProc,0,TIME_PERIODIC);
 }
}

void _declspec(dllexport) _cdecl DLO_done(void) {
 if (!usbHandle) return;
 timeKillEvent(TimerId);
 timeEndPeriod(10);
 CloseHandle(o.hEvent);
 CloseHandle(usbHandle);
 usbHandle = 0;
}

static BOOL CALLBACK ConfigDlgProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
 switch (Msg) {
  case WM_INITDIALOG: {
// Show current state of Usb2Prn found
   CheckDlgButton(Wnd,11,usbHandle?BST_CHECKED:BST_UNCHECKED);
   SetDlgItemInt(Wnd,12,DevNum,FALSE);
  }return TRUE;

  case WM_COMMAND: switch (LOWORD(wParam)) {
   case 1:
   case 2: EndDialog(Wnd,wParam); break;
   case 10: {	// re-open the Usb2Prn (in case of some disconnection meanwhile)
    int OldNum=DevNum;
    DLO_done();
    DevNum=GetDlgItemInt(Wnd,12,NULL,FALSE);
    DLO_init();
    CheckDlgButton(Wnd,11,usbHandle?BST_CHECKED:BST_UNCHECKED);
    if (usbHandle && DevNum!=OldNum) {
     char NewName[MAX_PATH], *p;
     p=GetNumberPointer(NewName,sizeof(NewName));
     if (p) {
      char DllName[MAX_PATH];
      lstrcpy(DllName,NewName);
      wnsprintf(p,NewName+MAX_PATH-p," %d.dll",DevNum);
      MoveFileEx(DllName,NewName,MOVEFILE_REPLACE_EXISTING);
     }
    }
   }break;
  }break;
 }
 return FALSE;
}

void _declspec(dllexport) _cdecl DLO_about(void) {
 DialogBox(hInst,MAKEINTRESOURCE(100),0,ConfigDlgProc);
}

/********************
 * main entry point *
 ********************/
void CALLBACK _DllMainCRTStartup(HINSTANCE hInstDll,DWORD fdwReason,LPVOID lpReserved) {
 if (fdwReason==DLL_PROCESS_ATTACH) {
  char DllName[MAX_PATH], *p;
  hInst = hInstDll;	// Where the (dialog) resource comes from - and the file name
  p=GetNumberPointer(DllName,sizeof(DllName));
  if (p) DevNum = StrToInt(++p);
  DisableThreadLibraryCalls(hInstDll);	// returns TRUE to continue DLL loading
 }
}
