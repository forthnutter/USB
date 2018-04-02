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
#define PCK 0	// (5) parallel clock, L->H edge for '595 port expanders
#define SCK 4	// (3) serial clock, L->H edge for '595 port expanders
#define SDO 3	// (2) serial data output for '595 port expanders

PROGMEM char usbHidReportDescriptor[24] = {    /* USB report descriptor */
 0x06, 0x00, 0xff,	// USAGE_PAGE (Generic Desktop)
 0x09, 0x01,		// USAGE (Vendor Usage 1)
 0xa1, 0x01,		// COLLECTION (Application)
 0x15, 0x00,		//   LOGICAL_MINIMUM (0) = LED off
 0x25, 0x01,		//   LOGICAL_MAXIMUM (1) = LED on
 0x75, 0x01,		//   REPORT_SIZE (1) = 1 bit (on/off)
 0x85, 0x02,		//   REPORT_ID (2) 
 0x96, 0x20, 0x00,	//   REPORT_COUNT (32) = 32 LEDs (packed in 4 bytes)
			//   prepared for more than 127 LEDs (16 bit count)
 0x09, 0x00,		//   USAGE (Undefined)
 0xb2, 0x02, 0x01,	//   FEATURE (Data,Var,Abs,Buf)
 0xc0};			// END_COLLECTION
// Report ID 1 is reserved for setting / retrieving the LED count,
// thus changing this descriptor (is this valid?)
// Report ID 3 will set a single LED, report ID 4 will reset a single LED.
// This is merely for the Pick-By-Light project, not for DiscoLitez.

/* First byte contains report ID (2), the next four the 32 LED states */
static uchar reportBuffer[5];
register uchar offset asm("r3");// where to write incoming data at usbFunctionWrite()

// A status variable that survives RESET
register uchar LedOn asm("r2");	// TRUE when LEDs are not switched OFF
register uchar LedOO asm("r4");	// temporary LED on/off register
register  char AutoT asm("r5");	// auto-timer for automatic animation

// shifts out a byte MSB first
static void OutByte(uchar b) {
 uchar t=0;		// Default value with SCK=low
 asm volatile(		// This routine needs 5 clocks per bit and no SPI
"	or	r4,%0	\n"	// set some LedOO bits
"	bst	%0,7	\n"
"	bld	%1,%4	\n"
"	out	%2,%1	\n"	// SCK = low, SDO = data bit (same time)
"	bst	%0,6	\n"
"	sbi	%2,%3	\n"	// SCK = high, clocking (3 clocks later)
"	bld	%1,%4	\n"
"	out	%2,%1	\n"	// SCK = low (2 clocks later)
"	bst	%0,5	\n"
"	sbi	%2,%3	\n"
"	bld	%1,%4	\n"
"	out	%2,%1	\n"
"	bst	%0,4	\n"
"	sbi	%2,%3	\n"
"	bld	%1,%4	\n"
"	out	%2,%1	\n"
"	bst	%0,3	\n"
"	sbi	%2,%3	\n"
"	bld	%1,%4	\n"
"	out	%2,%1	\n"
"	bst	%0,2	\n"
"	sbi	%2,%3	\n"
"	bld	%1,%4	\n"
"	out	%2,%1	\n"
"	bst	%0,1	\n"
"	sbi	%2,%3	\n"
"	bld	%1,%4	\n"
"	out	%2,%1	\n"
"	bst	%0,0	\n"
"	sbi	%2,%3	\n"
"	bld	%1,%4	\n"
"	out	%2,%1	\n"
"	nop		\n"
"	sbi	%2,%3	\n"
:
: "r" (b), "r" (t), "I" (_SFR_IO_ADDR(PORTB)), "I" (SCK), "I" (SDO)
: "cc");
}

static void PulsePck(void) {
 PORTB|=1<<PCK;			// strobe shifted data to outputs
 asm volatile("	nop\n"::);
 PORTB=0;
 LedOn=LedOO;			// Now the state of LEDs is safe
}

static void OutLedState() {
 LedOO = 0;
 OutByte(reportBuffer[4]);	// Start with farmost data
 OutByte(reportBuffer[3]);
 OutByte(reportBuffer[2]);
 OutByte(reportBuffer[1]);
 PulsePck();
}

static void OutLedOff() {	// needed for USB standby mode
 uchar i;			// Let compiler decide to unroll this loop!
 LedOO = 0;
 for (i=0; i<4; i++) OutByte(0);
 PulsePck();
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
 usbRequest_t *rq = (void*)data;

 usbMsgPtr = reportBuffer;
 if (rq->bRequest == USBRQ_HID_GET_REPORT) {  // wValue: ReportType:ReportID
  if (AutoT<0) OutLedState();
  AutoT=12;
  return sizeof(reportBuffer);
 }else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
  offset=0;
  return USB_NO_MSG;	// let usbFunctionWrite accept data
 }
 return 0;
}

// Data arrives in 8-byte chunks maximum (which is not a problem for Deluxe32)
uchar usbFunctionWrite(uchar *data, uchar len) {
 uchar space=sizeof(reportBuffer)-offset;
 if (len>space) len=space;	// ignore additional data
 memcpy(reportBuffer+offset,data,len);
 offset+=len;
 space-=len;
 if (!space) {
  OutLedState();
  AutoT=12;			// no auto-animation for ca. 3 seconds
  return 1;
 }
 return 0;		// expect more data
}

// Einfache Animation: Durchlaufender Leuchtpunkt
static void AutoPlay(void) {
 --AutoT;
 if (AutoT >= 0) return;
 if (AutoT == -40) {	// no LED lights up for 8 states
  PORTB|=1<<SDO;	// switch on _one_ LED
  LedOO|=AutoT;		// Ensure LedOO != 0
  AutoT=0;
 }
 PORTB|=1<<SCK;		// Shift only one bit
 PulsePck();		// update LEDs
}

// For performance of ISR, the oscillator tuning is done in main loop,
// not inside the ISR.
static void tuneOscillator(void) {
#define SOLL 14	// usbSofDiff should be 16500/8%256 = 14.5
#define MABW 7	// Deviation of less than ±6 is OK and silently ignored
 schar t = usbSofDiff-SOLL;	// center result around signed zero
 if (t<-MABW) OSCCAL++;		// make faster
 if (t> MABW) OSCCAL--;		// make slower
}
/* ------------------------------------------------------------------------- */

int main(void) __attribute__((__noreturn__));
int main(void) {
 uchar SofCmp = 0;
 uchar mcusr = MCUSR;
 MCUSR = 0;
 ACSR |= 0x80;		// disable analog comparator and save 70 µA
 DDRB=1<<PCK|1<<SCK|1<<SDO;	// Generate 3 SPI outputs
 WDTCR = 0x18;		// watchdog survives RESET: disable it
 WDTCR = 0;		// (not using macros to save some bytes)
 if (mcusr & 1<<WDRF && USBIN&USBMASK) {	// no SE0 (wakeup) state?
  if (LedOn) OutLedOff();
  GIMSK = 1<<INT0;
  MCUCR = 0xF4;		// SLEEP_MODE_PWR_DOWN
  MCUCR = 0xF0;		// switch off BOD too do save more power
  sei();		// must be enabled for wakeup
  sleep_cpu();		// stop all and let INT0 do the next wakeup
  cli();
 }
 MCUCR = 0x60;		// SLEEP_MODE_STANDBY (no need for pull-ups)
 PRR    = 0x0B;		// save power, all peripherals OFF except TIMER0
 TCCR0B = 0x02;		// TIMER0 runs with 1/8 CPU clock
 OutLedOff();
 AutoT = 0;		// animate immediately (visible after 10 seconds)
 WDTCR = 0x08;		// enable watchdog timer with shortest timeout (15 ms)
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
   SofCmp=t;
   tuneOscillator();
   if (!t) AutoPlay();	// Animate LEDs each 256 ms
  }
 }
}
