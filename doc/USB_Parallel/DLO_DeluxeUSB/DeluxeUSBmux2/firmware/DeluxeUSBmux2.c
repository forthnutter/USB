/* Firmware for DeluxeUSB hardware */
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include <string.h>
#include "usbdrv.h"
#include "usbdrv.c"

/* port pin definitions */
#define A PORTB	// (12)..(19) 8 Anodes (7 actually used)
#define K PORTD	// 5 Cathodes
#define K0 5	// (9)
#define K1 0	// (2)
#define K2 1	// (3)
#define K3 4	// (8)
#define K4 6	// (11)

PROGMEM char usbHidReportDescriptor[24] = {    /* USB report descriptor */
 0x06, 0x00, 0xff,	// USAGE_PAGE (Generic Desktop)
 0x09, 0x01,		// USAGE (Vendor Usage 1)
 0xa1, 0x01,		// COLLECTION (Application)
 0x15, 0x00,		//   LOGICAL_MINIMUM (0) = LED off
 0x25, 0x01,		//   LOGICAL_MAXIMUM (1) = LED on
 0x75, 0x01,		//   REPORT_SIZE (1) = 1 bit (on/off)
 0x85, 0x02,		//   REPORT_ID (2) 
 0x96, 0x20, 0x00,	//   REPORT_COUNT (40) = 40 LEDs (packed in 5 bytes)
 0x09, 0x00,		//   USAGE (Undefined)
 0xb2, 0x02, 0x01,	//   FEATURE (Data,Var,Abs,Buf)
 0xc0};			// END_COLLECTION
// Report ID 1 is reserved for setting / retrieving the LED count,
// thus changing this descriptor (is this valid?)
// Report ID 3 will set a single LED, report ID 4 will reset a single LED.
// This is merely for the Pick-By-Light project, not for DiscoLitez.

/* First byte contains report ID (2), the next five the 40 LED states */
static uchar reportBuffer[6];
register uchar offset asm("r3");// where to write incoming data at usbFunctionWrite()


// A status variable that survives RESET
register  char AutoT asm("r5");	// auto-timer for automatic animation
register uchar MuxK  asm("r6");	// current cathode, 0 = off
// Should be called at every SOF (1 ms) with low jitter
// Maybe a candidate for a timer ISR to be executed when USB traffic is least probably
static void DoMux(void) {
 static const PROGMEM uchar kat[6]={
   0x73,0x73^1<<K0,0x73^1<<K1,0x73^1<<K2,0x73^1<<K3,0x73^1<<K4};
 if (!MuxK) return;
 A = 0;
 MuxK++; if (MuxK>=6) MuxK=1;
 K = pgm_read_byte(kat+MuxK);
 A = reportBuffer[MuxK];
}
/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
 usbRequest_t *rq = (void*)data;

 usbMsgPtr = reportBuffer;
 if (rq->bRequest == USBRQ_HID_GET_REPORT) {  // wValue: ReportType:ReportID
//  AutoT=12;
//  reportBuffer[5]=0;
  return sizeof(reportBuffer)-1;
 }else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
  offset=0;
  return USB_NO_MSG;	// let usbFunctionWrite accept data
 }
 return 0;
}

// Data arrives in 8-byte chunks maximum
uchar usbFunctionWrite(uchar *data, uchar len) {
 uchar space=sizeof(reportBuffer)-1-offset;
 if (len>space) len=space;	// ignore additional data
 memcpy(reportBuffer+offset,data,len);
 offset+=len;
 space-=len;
 if (!space) {
//  AutoT=12;		// no auto-animation for ca. 3 seconds
  if (!MuxK) MuxK++;	// start multiplexing
//  reportBuffer[5]=0;
  return 1;
 }
 return 0;		// expect more data
}

// Einfache Animation: Laufender Leuchtpunkt in unterster Zeile
static void AutoPlay(void) {
 --AutoT;
 if (AutoT >= 0) return;
 reportBuffer[5]<<=1;	// shift lights
 if (AutoT == -16) {	// no LED lights up for 8 states
  reportBuffer[5]=1;	// switch on _one_ LED
  if (!MuxK) MuxK++;	// Ensure != 0
  AutoT=0;
 }
}

static void OutLedOff() {
 MuxK = 0;
 K = 0x73;
}
/* ------------------------------------------------------------------------- */

int main(void) __attribute__((__noreturn__));
int main(void) {
 uchar SofCmp = 0;
 uchar mcusr = MCUSR;
 MCUSR = 0;
 ACSR |= 0x80;		// disable analog comparator and save 70 µA
 DDRB = 0xFF;		// Enable anodes
 K = 0x73;
 DDRD=1<<K0|1<<K1|1<<K2|1<<K3|1<<K4;	// Generate cathode's outputs
 WDTCSR = 0x18;		// watchdog survives RESET: disable it
 WDTCSR = 0;		// (not using macros to save some bytes)
 if (mcusr & 1<<WDRF && USBIN&USBMASK) {	// no SE0 (wakeup) state?
  if (MuxK) OutLedOff();
  GIMSK = 1<<INT1;
  MCUCR = 0x30;		// SLEEP_MODE_PWR_DOWN
  sei();		// must be enabled for wakeup
  sleep_cpu();		// stop all and let INT1 do the next wakeup
  cli();
 }
 MCUCR = 0x20;		// SLEEP_MODE_STANDBY (no need for pull-ups)
 AutoT = 0;		// animate immediately (visible after 10 seconds)
 WDTCSR = 0x08;		// enable watchdog timer with shortest timeout (15 ms)
 usbInit();
 sei();

 for(;;){                /* main event loop */
  uchar t,d;
  if (USBIN&USBMASK) sleep_cpu();	// sleep, except at SE0, until SOF
  t=usbSofCount;	// atomic access to volatile variable
  d=t-SofCmp;		// time difference [ms], mostly 1
  if (d || !(USBIN&USBMASK)) wdt_reset();
  usbPoll();
  if (d) {		// After 1 ms (or maybe more)
   DoMux();
   SofCmp=t;
   if (!t) AutoPlay();	// Animate LEDs each 256 ms
  }
 }
}
