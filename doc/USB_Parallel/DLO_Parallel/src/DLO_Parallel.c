/* Discolitez plugin, for controlling eight channels via
 * an old-style parallel port (8 bit)
 *
 * This plugin takes the port address from library name!
 * The address must be (hexa)decimal encoded after a blank.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <shlwapi.h>
typedef enum {false,true} bool;
#define nobreak

WORD CurAddr;
BYTE PortState;
HINSTANCE hInst;
UINT TimerId;
HINSTANCE hInpOutDll;
void (_stdcall*Out32)(WORD a, BYTE b);

static const WORD DefAddrs[]={0x378,0x278,0x3BC};

// Fills combo box with 3 typical parallel port addresses
static void FillComboAddrs(HWND Wnd) {
 int i;
 bool set=false;
 char buf[20];
 for (i=0; i<3; i++) {
  wnsprintf(buf,sizeof(buf),"%Xh (%u, LPT%u)",DefAddrs[i],DefAddrs[i],i+1);
  ComboBox_AddString(Wnd,buf);
  if (CurAddr==DefAddrs[i]) {
   ComboBox_SetCurSel(Wnd,i);
   set=true;
  }
 }
 if (!set) {
  wnsprintf(buf,sizeof(buf),"%03Xh",CurAddr);
  SetWindowText(Wnd,buf);
 }
}

// Retrieves the DLL file name into buffer and returns a pointer
// to the first ' ' or the ".dll" portion of the file name.
// Returns NULL when failing.
static PSTR GetNumberPointer(PSTR p, size_t cch) {
 PSTR q;
 if (!GetModuleFileName(hInst,p,cch)) return NULL;
 p=PathFindFileName(p);
 if (!p) return p;
 q=StrChr(p,' ');
 if (!q) q=PathFindExtension(p);
 return q;
}

static bool SaveAddr(HWND Wnd) {
 char buf[16], DllName[MAX_PATH], NewName[MAX_PATH], *p;
 int a;
 buf[0]='0';
 buf[1]='x';
 GetWindowText(Wnd,buf+2,sizeof(buf)-2);
 if (!StrToIntEx(buf,STIF_SUPPORT_HEX,&a)) return false;
 if (HIWORD(a)) return false;	// >FFFFh
 if (!HIBYTE(a)) return false;	// <100h
 if (CurAddr==(WORD)a) return true;	// no change
 CurAddr=(WORD)a;	// assign
 p=GetNumberPointer(NewName,sizeof(NewName));
 if (!p) return true;
 lstrcpy(DllName,NewName);
 wnsprintf(p,NewName+MAX_PATH-p," 0x%X.dll",a);
 MoveFileEx(DllName,NewName,MOVEFILE_REPLACE_EXISTING);
 return true;
}

BOOL CALLBACK ConfigDlgProc(HWND Wnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
 switch (Msg) {
  case WM_INITDIALOG: {
   FillComboAddrs(GetDlgItem(Wnd,10));
  }return TRUE;

  case WM_COMMAND: switch (LOWORD(wParam)) {
   case 1: {
    HWND w=GetDlgItem(Wnd,10);
    if (!SaveAddr(w)) {
     char s[256];
     LoadString(hInst,1,s,sizeof(s));
     MessageBox(Wnd,s,NULL,MB_OK);
     SetFocus(w);
     break;
    }
   }nobreak;
   case 2: EndDialog(Wnd,wParam); break;
  }break;
 }
 return FALSE;
}

// Let a timer procedure actualize the output,
// reduce load due to too many kernel<->user mode transitions
static void CALLBACK TimerProc(UINT TimerId, UINT uMsg, DWORD dwUser,
  DWORD dw1, DWORD dw2) {
 static BYTE NewState;
 if (NewState!=PortState) Out32(CurAddr,NewState=PortState);
}

/**********************
 * exported functions *
 **********************/
BYTE _declspec(dllexport) _cdecl DLO_getnch(void) {
 return 8;
}

// Brain-dead construcion of DiscoLitez requires bad programming style here
LPSTR _declspec(dllexport) _cdecl DLO_getname(void) {
 static char buf[32];
 wnsprintf(buf,sizeof(buf),"Parallel port at %Xh",CurAddr);
 return buf;
}

void _declspec(dllexport) _cdecl DLO_datain(BYTE ch, BYTE val) {
 BYTE mask = 1<<(ch-1);
 if (val) {
  if (PortState&mask) return;
 }else{
  if (~PortState&mask) return;
 }
 PortState ^= mask;	// The timer procedure will output this value
}

void _declspec(dllexport) _cdecl DLO_init(void) {
// cannot link statically to inpout32.dll because this DLL should not be in PATH
// and therefore cannot be found if not in current directory of calling process
 char LibPath[MAX_PATH],*p;
 if (!GetModuleFileName(hInst,LibPath,sizeof(LibPath))) return;
 p=PathFindFileName(LibPath);
 if (!p) return;
 lstrcpyn(p,"inpout32.dll",LibPath+sizeof(LibPath)-p);
 hInpOutDll=LoadLibraryEx(LibPath,0,LOAD_WITH_ALTERED_SEARCH_PATH);
 if (!hInpOutDll) return;
 (FARPROC)Out32=GetProcAddress(hInpOutDll,MAKEINTRESOURCE(2));
 if (!Out32) return;
 timeBeginPeriod(10);
 TimerId=timeSetEvent(10,10,TimerProc,0,TIME_PERIODIC);
}

void _declspec(dllexport) _cdecl DLO_done(void) {
 timeKillEvent(TimerId);
 timeEndPeriod(10);
 FreeLibrary(hInpOutDll);
}

void _declspec(dllexport) _cdecl DLO_about(void) {
 DialogBox(hInst,MAKEINTRESOURCE(100),0,ConfigDlgProc);
}

/********************
 * main entry point *
 ********************/
BOOL CALLBACK _DllMainCRTStartup(HINSTANCE hInstDll,DWORD fdwReason,LPVOID lpReserved) {
 if (fdwReason==DLL_PROCESS_ATTACH) {
  char DllName[MAX_PATH], *p;
  int a;
  hInst = hInstDll;	// Where the (dialog) resource comes from - and the file name
  DisableThreadLibraryCalls(hInstDll);
  p=GetNumberPointer(DllName,sizeof(DllName));
  CurAddr = p
	 && StrToIntEx(p+1,STIF_SUPPORT_HEX,&a)
	 && !HIWORD(a)
	 && HIBYTE(a) ? (WORD)a : 0x378;
 }
 return TRUE;
}
