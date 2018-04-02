/* Discolitez plugin, for controlling 32 channels via
 * DeluxeUSB device containing ATtiny25 and 4 74HC595
 * See schematic and firmware for this adapter!
 * Alternatively useable for DeluxeUSBmux circuitry
 * and other firmware-compatible output devices.
 *
 * This plugin uses the first DeluxeUSB adapter found
 * if zero-based number is not given by DLL file name suffix.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <hidpi.h>

// Opens the n'th (zero-based) HID device that has VID=16C0, PID=05DF,
// and contains L"DeluxeUSB" somewhere in its Device Name string
// Use CloseHandle() to free the handle returned.
// Returns 0 when failing.
static HANDLE OpenUsbHid(int n) {
 HANDLE h=0;
 DWORD i;
 HDEVINFO devs;
 GUID hidGuid;		// GUID for HID driver

 HidD_GetHidGuid(&hidGuid);
 devs = SetupDiGetClassDevs(&hidGuid, NULL, 0, DIGCF_PRESENT|DIGCF_DEVICEINTERFACE);
 if (devs==INVALID_HANDLE_VALUE) return h;

 for (i=0;;i++) {
  DWORD size = 0;
  PSP_DEVICE_INTERFACE_DETAIL_DATA interface_detail;
  union{
   struct{
    SP_DEVICE_INTERFACE_DATA devinterface;
    SP_DEVINFO_DATA devinfo;
   };
   HIDD_ATTRIBUTES deviceAttributes;
   WCHAR productName[32];
  }s;	// some shared buffers not used the same time (does the optimizer see that?)

  if (h) CloseHandle(h), h=0;
  s.devinterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
  if (!SetupDiEnumDeviceInterfaces(devs, 0, &hidGuid, i, &s.devinterface)) break;
	// See how large a buffer we require for the device interface details
  SetupDiGetDeviceInterfaceDetail(devs, &s.devinterface, NULL, 0, &size, 0);
  s.devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
  interface_detail = LocalAlloc(LPTR,size);
  if (!interface_detail) continue;
  interface_detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
  s.devinfo.cbSize = sizeof(SP_DEVINFO_DATA);
  if (!SetupDiGetDeviceInterfaceDetail(devs, &s.devinterface, interface_detail, size, 0, &s.devinfo)) {
   LocalFree(interface_detail);	// ignore this entry in case of error
   continue;
  }
  h = CreateFile(interface_detail->DevicePath, GENERIC_READ|GENERIC_WRITE,
    FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  LocalFree(interface_detail);
  if (h == INVALID_HANDLE_VALUE) h = 0;
  if (!h) continue;
  if (!HidD_GetAttributes(h, &s.deviceAttributes)) continue;
  if (*(DWORD*)&s.deviceAttributes.VendorID != 0x05DF16C0) continue;	// not mine *)
  if (!HidD_GetProductString(h, s.productName, sizeof(s.productName))) continue;
  if (!StrStrW(s.productName,L"DeluxeUSB")) continue;	// not mine
  if (!n) break;	// found my device
  n--;			// iterate to my next device
 }
 SetupDiDestroyDeviceInfoList(devs);
 return h;
}

static HANDLE usbHandle;// from OpenUsbHid()
static DWORD PortState;	// current state of 32 LEDs
static int DevNum;	// zero-based device number
static UINT TimerId;	// from timeSetEvent()
static HINSTANCE hInst;	// the DLL's instance handle

// Sets the 32 LED states to given new state
static void OutPortState(DWORD State) {
 if (usbHandle) {
  BYTE report[5];
  report[0] = 2;		// report ID
  *(DWORD*)(report+1) = State;	// report data *)
  HidD_SetFeature(usbHandle,report,sizeof(report));
 }
} // *) The Intel architecture permits unaligned access

// The configuration dialog is merely an about box
// that shows the current state of USB-HID connection.
static BOOL CALLBACK ConfigDlgProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
 switch (Msg) {
  case WM_INITDIALOG: {
// Show current state of DeluxeUSB found
   CheckDlgButton(Wnd,11,(UINT)usbHandle);
  }return TRUE;

  case WM_COMMAND: switch (LOWORD(wParam)) {
   case 1:
   case 2: EndDialog(Wnd,wParam); break;
   case 10: {	// re-open the DeluxeUSB (in case of some disconnection meanwhile)
    if (usbHandle) CloseHandle(usbHandle);
    usbHandle=OpenUsbHid(DevNum);
    CheckDlgButton(Wnd,11,(UINT)usbHandle);
   }break;
  }break;
 }
 return FALSE;
}

// Let a timer procedure actualize the output,
// reduce load due to too many kernel<->user mode transitions
static void CALLBACK TimerProc(UINT TimerId, UINT uMsg, DWORD dwUser,
  DWORD dw1, DWORD dw2) {
 static DWORD NewState;
 if (NewState!=PortState) OutPortState(NewState=PortState);
}

/**********************
 * exported functions *
 **********************/
BYTE _declspec(dllexport) _cdecl DLO_getnch(void) {
 return 32;
}

LPSTR _declspec(dllexport) _cdecl DLO_getname(void) {
 if (DevNum) {
  static char buf[32];
  wnsprintf(buf,sizeof(buf),"DeluxeUSB device #%d",DevNum);
  return buf;
 }
 return "DeluxeUSB device";
}

void _declspec(dllexport) _cdecl DLO_datain(BYTE ch, BYTE val) {
 DWORD mask = 1<<(ch-1);
 if (val) {
  if (PortState&mask) return;
 }else{
  if (~PortState&mask) return;
 }
 PortState ^= mask;	// The timer procedure will output this value later
}

void _declspec(dllexport) _cdecl DLO_init(void) {
 usbHandle=OpenUsbHid(DevNum);
 OutPortState(0);	// At startup, switch all LEDs off
 timeBeginPeriod(10);
 TimerId = timeSetEvent(10,10,TimerProc,0,TIME_PERIODIC);
}

void _declspec(dllexport) _cdecl DLO_done(void) {
 timeKillEvent(TimerId);
 timeEndPeriod(10);
 if (usbHandle) CloseHandle(usbHandle);
 usbHandle = 0;
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
  if (GetModuleFileName(hInstDll,DllName,sizeof(DllName))
  && (p=StrChr(PathFindFileName(DllName),' ')))
    DevNum = StrToInt(p);
  DisableThreadLibraryCalls(hInstDll);	// returns TRUE to continue DLL loading
 }
}
