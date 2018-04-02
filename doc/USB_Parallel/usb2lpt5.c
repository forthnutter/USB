/* Firmware für ATmega48 im USB2LPT 1.5, haftmann#software 11/10
 * basierend auf: »usbdrv« von Objective Development GmbH (Österreich),
 * Beispieldatei von Henning Paul (einige Setup-Transfers)
 * Lizenz: GNU GPL v2 (see License.txt)
 * Zu übersetzen und zu brennen mit zugehörigem »makefile«,
 * bspw. »make« zum Kompilieren, »make flash« zum Brennen (ggf. mit Kompilation),
 * (einmalig pro Chip) »make fuse« zum Setzen der Sicherungen (Taktquelle u.ä.)
 * Das Paket »winavr« ist erforderlich!
 * Alle Schiebeoperationen (>>, <<) sind wegen Compiler-Umständen (int-Cast)
 * in Schiebe-Zuweisungen (>>=, <<=) umgesetzt;
 * dito auch & und | bei Tests (bspw. bei »if«) mit mehr als einem Bit.
 *
 * Mit dem Quarz ist die Firmware-Größe kleiner und die Arbeitsfrequenz
 * unabhängig von Betriebsspannungsschwankungen.
 * Alle Portpins sind hierbei belegt.
 *
 * Im USB-Schlafmodus wird per Pegelwechsel-Interrupt aufgeweckt.

*101125	Firmware angepasst für SETUP-Transfers (statt BULK) für Windows Vista/7
	sowie Linux (wie 1.6), Kodegröße unter 4 KByte gehalten
-130507	Kein Linkerskript, sondern -nostartfiles wie USB2LPT6
+130507	Umschalter Bulk/Interrupt, Deskriptoren im EEPROM
+130508	Opcodes für Bitmanipulation
 */

// Aus »winavr«
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include <string.h>	// memcpy
#include <util/delay.h>	// _delay_us

// Aus »usbdrv«
#include "usbdrv.c"

// Firmware-Datum (Meldung bei A0-Request mit wData==6 und wLength==2)
#define DATEYEAR	2013
#define DATEMONTH	5
#define DATEDAY		28

/************************
 * Hardware:
 *
 * PortB = (zumeist) Steuerport
 *[12] ICP   0 = USB D- (sowie Interrupt und Pullup 10k nach 5P)
 *[13] OC1   1 = USB D+
 *[14] SS    2 = /STB - /C0 (1)
 *[15] MOSI  3 = /AF  - /C1 (14)
 *[16] MISO  4 = /INI -  C2 (16)
 *[17] SCK   5 = /SEL - /C3 (17)
 * [8] XTAL2 6 = Quarz
 * [7] XTAL1 7 = Quarz
 *
 * PortC = (zumeist) Statusport
 *[23] ADC0  0 = /LED (low-aktiver Ausgang)
 *[24] ADC1  1 = /ERR -  S3 (15)
 *[25] ADC2  2 =  ONL -  S4 (13)
 *[26] ADC3  3 =  PE  -  S5 (12)
 *[27] ADC4  4 = /ACK -  S6 (10)
 *[28] ADC5  5 =  BSY - /S7 (11)
 *[29] RESET 6 = /Reset (kein E/A-Anschluss) - Lötbrücke nach ONL S4 (13)
 *
 * PortD = Datenport
 *[30] RxD   0 = D0 (2)
 *[31] TxD   1 = D1 (3)
 *[32] INT0  2 = D2 (4)
 * [1] INT1  3 = D3 (5)
 * [2] T0    4 = D4 (6)
 * [9] T1    5 = D5 (7)
 *[10] AIN0  6 = D6 (8)
 *[11] AIN1  7 = D7 (9)
 *
 * In eckigen Klammern: ATmega48-Pinnummer (TQFP-Gehäuse)
 * In runden Klammern: Pinnummer SubD25-Buchse
 * Übrige Pins:
 *[4][6][18] - Betriebsspannung (5V, nicht 3,3V, wegen Ausgabe auf SubD)
 *[3][5][21] - Masse
 *[19][20][22] - zusätzliche A/D-Wandler-Eingänge; A/D-Referenzspannung
 *(18)..(24) - Masse
 *(25) - Masse oder umlötbar 5V
 *
 * Die Zuordnung zu den Portpins erfolgte nach der Maßgabe,
 * Portadressen nicht aufzuteilen, um bei Ausgaben mit zwei Pegelwechseln
 * diese Pegelwechsel exakt gleichzeitig erscheinen zu lassen.
 * Zur Ausrichtung von Ein-Ausgabedaten genügen Schiebebefehle.
 * Damit war PortD als Datenport zwangsweise festgelegt, und INT0 (INT1)
 * nicht für USB nutzbar.
 ************************/
 
#define FATDATE (((DATEYEAR)-1980)*512+(DATEMONTH)*32+(DATEDAY))
#define BCDDATE (((DATEYEAR)-2000)/10<<12)+(((DATEYEAR)-2000)%10<<8)+((DATEMONTH)/10<<4)+(DATEMONTH)%10
 
inline __attribute((naked,section(".vectors"))) void __vectors() {
 asm volatile(
"	ldi	r30,0x5F\n"	// initialize stack
"	ldi	r31,0x02\n"
"	rjmp	main\n"
"	rjmp	__vector_3\n"	// das FAT-Datum wird simuliert
"	.byte	%0,%1\n"	// FAT-Datum (an "falscher" Adresse 8)
	:
	:"M" (FATDATE&0xFF),
	 "M" ((FATDATE>>8)&0xFF)
	);
}
void __do_copy_data(void) __attribute__((naked));
void __do_copy_data(void) {}	// no non-zero-initialized static data

void __do_clear_bss(void) __attribute__((naked));
void __do_clear_bss(void) {}	// no static data at all 
int main(void) __attribute__((noreturn,naked));

/* Parallelport */
register uchar data_byte	asm("r7");
register uchar DCR		asm("r6");
			// Device Control Register (SPP +2)
#define DIRECTION_INPUT (DCR&0x20)
register uchar ECR		asm("r5");
			// Extended Control Register (ECP +402)
register uchar ECR_Bits		asm("r4");
			// 1-aus-n-Code der höchsten 3 Bits aus ECR
register uchar EppTimeOut	asm("r3");
			// Bit0 = 0: kein Timeout aufgetreten
			// Bit2 = 0: Interrupt aufgetreten (zz. ungenutzt)
			// Alle anderen Bits müssen =1 sein!
register uchar Feature		asm("r2");
			// Bit0 = Offene-Senke-Simulation für Daten (SPP +0)
			// Bit2 = Offene-Senke-Simulation für Steuerport (+2)
			// Bit4 = Seriennummer via USB-Deskriptor
			// Bit6 = DirectIO (keine Invertierungen)
			// Bit7 = Bulk statt Interrupt
/* FIFO */
#define FIFOSIZE 16	// muss laut Programmlogik Vielfaches von 2 sein
static unsigned Fifo[FIFOSIZE] __attribute((section(".noinit")));
				//wegen ECP brauchen wir 9 bit Breite
register uchar fifor		asm("r8");
register uchar fifow		asm("r9");	//Indizes

/* sonstiges */
static uchar Led_T;	// "Nachblinkzeit" der LED in ms, 0 = LED blinkt nicht
static uchar Led_F;	// "Blinkfrequenz" in ms (halbe Periode)
static uchar Led_C;	// Blink-Zähler (läuft alles über SOF-Impuls)

// Ein WORD in FIFO schreiben - nichts tun wenn FIFO voll
static void PutFifo(unsigned w) {
 uchar idx, ecr = ECR;		// mit Registern (nicht RAM) arbeiten
 if (ecr & 2) return;		// nichts tun wenn FIFO voll
 idx = fifow;			// in Register holen
 Fifo[idx++] = w;		// einschreiben
 idx &= FIFOSIZE - 1;		// if (idx >= FIFOSIZE) idx = 0;
 ecr &= ~1;			// FIFO nicht leer
 if (idx == fifor) ecr |= 2;	// FIFO voll
 fifow = idx;			// Register rückspeichern
 ECR = ecr;
}

// ein WORD aus FIFO lesen - letztes WORD liefern wenn FIFO leer
// (Ein echtes ECP tut das auch!)
static unsigned GetFifo(void) {
 uchar idx = fifor, ecr = ECR;	// mit Registern (nicht RAM) arbeiten
 unsigned w;
 if (ecr & 1) {			// bei leerer FIFO vorhergehendes WORD liefern
  idx--;
  idx &= FIFOSIZE - 1;		// if (idx >= FIFOSIZE) idx = FIFOSIZE - 1;
  w = Fifo[idx];
 }else{
  w = Fifo[idx++];
  idx &= FIFOSIZE - 1;		// if (idx >= FIFOSIZE) idx = 0;
  ecr &= ~2;			// FIFO nicht voll
  if (fifow == idx) ecr |= 1;	// FIFO leer
  fifor = idx;			// Register rückspeichern
  ECR = ecr;
 }
 return w;
}

// Datenrichtungswechsel von DCR Bit 5 wirksam werden lassen,
// dabei Simulation der Offenen Senke (Open Collector) beachten
static void LptSetDataDir(void) {
 if (DIRECTION_INPUT) {
  DDRD  = 0x00;		// alles EINGÄNGE
  PORTD = 0xFF;		// alle Pull-Ups EIN
 }else{
  PORTD = data_byte;	// Datenbyte ausgeben
  DDRD  = Feature & 0x01 ? ~PORTD : 0xFF;
 }
}

// Aktivieren der Parallelschnittstelle bei USB-Aktivität
// oder Rückstellen beim Löschen des DirectIO-Bits im Feature-Register
static void LptOn(void) {
 uchar t;
 LptSetDataDir();	// Datenport: Ausgänge (oder nur Pullups)
 DDRC  = 0x01;		// Statusport: nur die LED ist Ausgang
 PORTC = 0x3E;		// Statusport: fünf Pullups
 t = DCR;
 t ^= 0x0B;
 t <<= 2;
 PORTB = t;
 DDRB  = 0x3C;		// USB-Anschlüsse bleiben Eingänge
}

// <strobe>-Low-Bits auf Steuerport ausgeben und max. 10 µs auf WAIT=H warten
// Liefert Daten eines READ-Zyklus'
// Ausgabedaten müssen vorher auf PORTD gelegt werden.
// Bei Fehler wird das TimeOut-Bit gesetzt
// (Gelöscht wird es durch Schreiben einer Null aufs Statusport.)
static uchar epp_io(uchar strobe) {
 uchar i, saveoe = DDRD, save = PORTB;
 if (PINC & 0x20) goto n_ok;	// BUSY
 PORTB = save & strobe;	// ASTROBE bzw. DSTROBE sowie ggf. WRITE auf LOW
 if (!(strobe & 0x04)) DDRD = 0xFF;	// Ausgabe (jetzt erst)
 i = 24; do{
  if (PINC & 0x20) goto okay;	// sbic PINC,5; rjmp okay (2)
 }while (--i);			// dec r18; brne l1 (3)
n_ok:
 EppTimeOut |= 0x01;		// TimeOut markieren
okay:
 i = PIND;			// Daten einlesen (nur für INPUT relevant:-)
 if (!(strobe & 0x04)) DDRD = saveoe;
 PORTB = save;
 return i;
}

// === OUT auf Adresse +0 (Datenport) ===
static void out0(uchar b) {
 uchar t = ECR_Bits & 0x4C;	// Compiler macht int-Murks ohne Hilfsvariable
 if (t) PutFifo(b);		// eine FIFO-Betriebsart
 else{
  data_byte = b;
  if (!DIRECTION_INPUT) {
   PORTD = b;
   if (Feature & 1) DDRD = ~b;	// Simulation OC (mit Pullups)
  }
 }
}

// === OUT auf Adresse +1 (Statusport) ===
// Eine Ausgabe erfolgt nur bei DirectIO = 1 oder bei (via out405)
// auf Ausgabe geschalteten Leitungen.
// Ansonsten keine Ausgabe; Pullups bleiben eingeschaltet
static void out1(uchar b) {
 if (ECR_Bits & 0x10 && !(b & 0x01)) EppTimeOut &=~ 0x01;	// TimeOut-Bit
 b >>= 2;
 if (!(Feature & 0x40)) {
  b ^= 0x20;		// BSY-Bit invertieren
  b |= ~DDRC;		// Ausgabe nur bei DirectIO oder vorhandenen Ausgabepins
 }
 b &= 0x3E;
 PORTC = (PORTC & 1) | b;
}

// === OUT auf Adresse +2 (Steuerport) ===
// Bit4=IRQ-Freigabe (nicht unterstützt, aber gespeichert)
// Bit5=Ausgabetreiber-Freigabe
static void out2(uchar b) {
 uchar t = ECR_Bits & 0x05;
 b |= 0xC0;
 if (t) b &=~ 0x20;	// Richtung auf Ausgabe fixieren
 t = b^DCR;	// geänderte Bits
 DCR = b;
 if (!(Feature & 0x40)) {	// kein DirectIO?
  if (t & 0x20) LptSetDataDir();// bei Richtungswechsel
  b ^= 0x0B;			// Invertierungen anpassen
 }
 b <<= 2;
 PORTB = b;
 if (Feature & 0x04) DDRB = ~b & 0xFC;	// USB-DDR-Bits müssen 0 bleiben
}

// === OUT auf Adresse +3 (EPP-Adresse) ===
static void out3(uchar b) {
 PORTD = b;
 epp_io(~0x24);			// AddrStrobe und Write LOW (0xx0)
}

// === OUT auf Adresse +4 (EPP-Daten) ===
static void out4(uchar b) {
 PORTD = b;
 epp_io(~0x0C);			// DataStrobe und Write LOW (xx00)
}

// === OUT auf Adresse +400 (ECP-Daten-FIFO) ===
static void out400(uchar b) {
 uchar t = ECR_Bits & 0x4C;
 if (t) PutFifo(b | 0x100);	// Datenbyte einschreiben
}

// === OUT auf Adresse +402 (ECP-Steuerport) ===
static void out402(uchar b) {	// svw. SetECR
 uchar t;
 ECR = (b & 0xF8) | 0x05;	// FIFOs leeren
 b >>= 5;
 t = 1;
 t <<= b;
 ECR_Bits = t;			// 1-aus-n-Kode setzen
 EppTimeOut = t & 0x10 ? 0xFE : 0xFF;	// Timeout-Bit voreinstellen
 if (t & 0x05) {		// SPP oder SPP-FIFO?
  if (DIRECTION_INPUT) {
   DCR &=~ 0x20;		// Richtung fest auf AUSGABE
   LptSetDataDir();
  }
 }
 fifor = fifow = 0;
}

// === OUT auf Adresse +404 (Datenrichtung Datenport) [HINTERTÜR] ===
static void out404(uchar b) {
 DDRD = b;
}

// === OUT auf Adresse +405 (Datenrichtung Statusport) [HINTERTÜR] ===
static void out405(uchar b) {
 b >>= 2;
 b |= 0xC1;
 DDRC = b;			// LED-Ausgang immer EIN
 if (!(Feature & 0x40)) {	// ohne DirectIO Pullups sicherstellen!
  PORTC |= ~b;
 }
}

// === OUT auf Adresse +406 (Datenrichtung Steuerport) [HINTERTÜR] ===
static void out406(uchar b) {
 b <<= 2;
 DDRB = b;			// USB-Anschlüsse stets Eingang
}

// === OUT auf Adresse +407 (USB2LPT-Feature-Register) [HINTERTÜR] ===
static void out407(uchar b) {
 uchar t;
 b &= 0xD5;			// nur unterstützte Bits durchlassen
 t = Feature ^ b;
 Feature = b;
 if (t) {
// BrennFeature()
  if (t & 0x01 && !(b & 0x40) && !DIRECTION_INPUT) {
   DDRD = b & 0x01 ? ~PORTD : 0xFF;
  }
  if (t & 0x04 && !(b & 0x40)) {
   DDRB = b & 0x04 ? ~PORTB & 0x3C : 0x3C;
  }
  if (t & 0x40 && !(b & 0x40)) {	// Portrichtungen wiederherstellen
   LptOn();
  }
 }
}
 
// Der Teufel hat ECP erfunden! Wie soll festgestellt werden, ob in der
// FIFO ein Adress- oder ein Datenbyte liegt?
// Mein "echtes" EC-Port ignoriert beim Rücklesen einfach das neunte Bit.
// === IN von Adresse +0 (Datenport) ===
static uchar in0(void) {
 uchar t = ECR_Bits & 0x4C;
 if (t) t = GetFifo();
 else t = PIND;			// ansonsten stets Portpins lesen
 return t;
}

// === IN von Adresse +1 (Statusport) ===
static uchar in1(void) {
 uchar t = PINC;
 t <<= 2;
 t |= 0x07;
 if (!(Feature & 0x40)) t ^= 0x80;
 t &= EppTimeOut;
 return t;
}

// b==0: Echtes PIN-Rüclesen, sonst PORT-Rücklesen
static uchar internal_in2(uchar b) {
 b = b ? PORTB : PINB;
 b >>= 2;
 b &= 0x0F;
 if (!(Feature & 0x40)) b ^= 0x0B;
 b |= DCR & 0xF0;
 return b;
}

// === IN von Adresse +2 (Steuerport) ===
// Überraschung: Bei einem "genügend neuen" echten Parallelport sind die
// Steuerleitungen gar nicht (mehr) rücklesbar!
// Hier wird diese Einschränkung nur in den 3 FIFO-Betriebsarten nachgeahmt
static uchar in2(void) {
 return internal_in2(ECR_Bits & 0x4C);
}

// === IN von Adresse +3 (EPP-Adresse) ===
static uchar in3(void) {
 return epp_io(~0x20);		// AddrStrobe LOW (0xxx)
}

// === IN von Adresse +4 (EPP-Daten) ===
static uchar in4(void) {
 return epp_io(~0x08);		// DataStrobe LOW (xx0x)
}

// === IN von Adresse +400 (ECP-FIFO) ===
static uchar in400(void) {
 uchar t = ECR_Bits & 0xCC;
 if (!t) return 0xFF;
 if (t & 0x80) return 0x10;	// Konfigurationsregister A (konstant)
 return GetFifo();
}

// === IN von Adresse +401 (ECP-???) ===
static uchar in401(void) {
 return ECR_Bits & 0x80 ? 0 : 0xFF;	// Konfigurationsregister A (0)
}

// === IN von Adresse +402 (ECP-Steuerport) ===
static uchar in402(void) {
 return ECR;
}

// === IN von Adresse +404 (Datenrichtung Datenport) [HINTERTÜR] ===
static uchar in404(void) {
 return DDRD;
}

// === IN von Adresse +405 (Datenrichtung Statusport) [HINTERTÜR] ===
static uchar in405(void) {
 uchar t = DDRC;
 t <<= 2;
 t &= 0xF8;
 return t;
}

// === IN von Adresse +406 (Datenrichtung Steuerport) [HINTERTÜR] ===
static uchar in406(void) {
 uchar t = DDRB;
 t >>= 2;
 t &= 0x0F;
 return t;
}

// === IN von Adresse +407 (USB2LPT-Feature-Register) [HINTERTÜR] ===
static uchar in407(void) {
 return Feature;
}

// === WARTEN (blockieren) in 4-µs-Stückelung ===
static void wait(uchar us) {
 us++; do _delay_us(4-3E6/F_CPU); while (--us);
}

// Zyklischer Aufruf; Emulation der SPP-Ausgabefifo
static void SppXfer(void) {
 if (ECR & 1) return;		// bei leerer FIFO nichts tun
 if (PINC & 0x20) return;	//  BSY = H: nichts tun
 PORTD = GetFifo();
 PORTB &= ~0x04;		// /STB = L
 wait(1);
 PORTB |=  0x04;		// /STB = H
}

// Zyklischer Aufruf; Emulation der ECP-Ein/Ausgabefifo
static void EcpXfer(void) {
 unsigned w;
 if (DIRECTION_INPUT) {		// FIFO-Eingabe
  if (ECR & 2) return;		// bei voller FIFO nichts tun
  if (!(PORTB & 0x08)) {	//  /AF = L: Byte einlesen?
   if (PINC & 0x10) return;	// /ACK = H: kein Byte von außen vorhanden
   PORTB |= 0x08;		//  /AF = H setzen
  }				//  /AF = H: hier zweite Handshake-Phase
  if (!(PINC & 0x10)) return;	// /ACK = L: nichts tun!
  w = PIND;
  if (PINC & 0x20) w|=0x0100;	//  BSY = Command(0) / Data(1)
  PutFifo(w);
  PORTB &= ~0x08;		//  /AF = L setzen
 }else{				// FIFO-Ausgabe
  if (PORTB & 0x04) {		// /STB = H: Byte ausgeben?
   uchar t;
   if (ECR & 1) return;		// bei leerer FIFO nichts tun
   if (PINC & 0x20) return;	//  BSY = H: nichts tun!
   w = GetFifo();		// Adress- oder Datenbyte lesen
   PORTD = (uchar)w;		// Byte anlegen
   t = PORTB & 0x34;
   if (w & 0xFF00) t |= 0x08;	// AF setzen oder nicht
   PORTB = t;
   PORTB &= ~0x04;		// /STB = L setzen
  }				// /STB = L: hier zweite Handshake-Phase
  if (!(PINC & 0x20)) return;	//  BSY = L: nichts tun!
  PORTB |= 0x04;		// /STB = H setzen
 }
}

// LED starten mit Blinken, t = Periodendauer
void Led_Start(uchar t) {
 if (!Led_T) {
  Led_C = t;
  PORTC |= 0x01;	// LED ausschalten
 }
 Led_T = Led_F = t;
}

// LED-Zustand alle 1 ms (SOF) aktualisieren
static void Led_On1ms(void) {
 uchar led_t = Led_T, led_c;
 if (!led_t) return;
 led_c = Led_C;		// in Register laden
 if (!--led_c) {
  PORTC ^= 0x01;	// LED umschalten
  led_c = Led_F;	// Zähler neu laden
 }
 if (!--led_t) PORTC &= ~0x01;	// LED einschalten
 Led_T = led_t;		// Register rückschreiben
 Led_C = led_c;
}

static void initIOPorts(void) {
 data_byte = 0x00;
 DCR = 0xCC;
}

static uchar getbyte(uchar a) {
 a&=0xF8;
 if (a==0x00) return in0();
 if (a==0x08) return PORTD;
 if (a==0x10) return in1();
 if (a==0x18) {a=PORTC; a<<=2; a|=7; return a;}
 if (a==0x20) return in2();
 if (a==0x28) return internal_in2(1);
 a&=0xF0;
 if (a==0xA0) return in402();
 if (a==0x30) return in3();
 if ((a&0xCF)==0x40) return in4();
 if (a==0x80) return in400();
 if (a==0x90) return in401();
 if (a==0xC0) return in404();
 if (a==0xD0) return in405();
 if (a==0xE0) return in406();
 if (a==0xF0) return in407();
 return 0xFF;
}

static void setbyte(uchar a, uchar b) {
 a&=0xF0;
 if (a==0x00) out0(b);
 else if (a==0x10) out1(b);
 else if (a==0x20) out2(b);
 else if (a==0xA0) out402(b);
 else if (a==0x30) out3(b);
 else if ((a&0xCF)==0x40) out4(b);
 else if (a==0x80) out400(b);
 else if (a==0xC0) out404(b);
 else if (a==0xD0) out405(b);
 else if (a==0xE0) out406(b);
 else if (a==0xF0) out407(b);
}

static void waitbit(uchar a, uchar m) {
 uchar t=usbSofCount;
 uchar x = 0;
 if (a&8) x--;
 while ((getbyte(a)^x)&m) if (t!=usbSofCount) break;
}

// Ausgabedaten bearbeiten und Ergebnisbytes in <buf> aufsammeln,
// liefert Anzahl der nach <buf> geschriebenen Bytes.
// Behandelt auch über Puffergrenzen hinauslaufende 2-Byte-Kommandos
static uchar ProcessInOut(const uchar *data, uchar len, uchar *buf) {
 static uchar PendingByte;
 uchar *InData = buf, command, value;
 if (!len) return 0;
 Led_Start(100);	// schnelles Blinken (5 Hz)
 do{
  command = PendingByte;
  if (command) {
   command &= ~0x10;	// eh' nur 2-Byte-Ausgabekommando
   PendingByte = 0;
   len++;
  }else command = *data++;
  if (command & 0x10) {	// IN-Kommandos
   *InData++ = getbyte(command<<4);	// IN-Kommandos erzeugen Antwort-Bytes
  }else{		// OUT-Kommandos mit Folgebyte
   if (!--len) {	// Wird erst beim nächsten ProcessInOut verarbeitet
    command |= 0x10;
    PendingByte = command;	// merken für Fortsetzung
    break;		// mit len==0 aus Schleife ausbrechen
   }
   value = *data++;
   if (command<0x10) setbyte(command<<4,value);
   else if (command==0x20) wait(value);
   else if (command>=0x21 && command<=0x24) {
    uchar m=1<<(value&7);
    if (command==0x24) waitbit(value,m);
    else{
     uchar b=getbyte(value);
     if (command<0x22) b|=m;
     else if (command==0x22) b&=~m;
     else b^=m;
     setbyte(value,b);
    }
   }
  }
 }while (--len);
 return InData - buf;	// Geschriebene Bytes liefern
}

// Antwort-Puffer (zurzeit nicht überlauf-geschützt!)
static uchar gReplyBuf[64], gReplyLen __attribute((section(".noinit")));
 
static uchar gbRequest;		// Kopie vom letzten SETUP-Paket
static size_t gAdr, gLen  __attribute((section(".noinit")));
		
//-090624: Ohne diese Maßnahme gelangen Daten „in den falschen Hals“
static uchar ExpectEP1OutToken=USBPID_DATA0;

USB_PUBLIC uchar usbFunctionSetup(uchar data[8]) {
 static uchar replyBuf[2] __attribute((section(".noinit")));
 
 gbRequest = data[1];		// SETUPDAT merken (wie Cypress-Controller)
 gAdr = ((usbRequest_t*)data)->wValue.word;
 gLen = ((usbRequest_t*)data)->wLength.word;

// Rudiment von Henning Paul, für seinen Linux-Treiber
 usbMsgPtr = replyBuf;
// SetFeature -> Stall -> EP1OUT: Data-Toggle rücksetzen
 if (data[0]==0x02 && data[1]==0x03 && data[2]==0x00 && data[4]==0x01) {
  ExpectEP1OutToken=USBPID_DATA0;
 }else	// Ballast!! Welchen Wert hat bmRequestType?
    if(data[1] == 0){       /* ECHO */
        replyBuf[0] = data[2];
        replyBuf[1] = data[3];
        return 2;
    }else if(data[1] == 1){       /* READ_REG */
	if (data[2] == 0){
            replyBuf[0] = in0();
            return 1;
	} 
	else if (data[2] == 1){
            replyBuf[0] = in1();
            return 1;
	}
	else if (data[2] == 2){
	    replyBuf[0] = in2();
            return 1;
	}
    }else if(data[1] == 2){       /* WRITE_REG */
	if (data[2] == 0){
		out0(data[4]);
        }
	else if (data[2] == 2){
		out2(data[4]);
        }
 }else if (data[0] == 0xC0) {	// IN, Vendor, Device
// Ehemalige BULK-Transfers
  if (data[1] >= 0x90 && data[1] <= 0x94) {
// bis zu 4 Bytes von SETUPDAT als OUT-Daten verarbeiten
// (Speed! - Für übliche geringe Ausgabemengen kein extra SETUP-Transfer)
   gReplyLen += ProcessInOut(data+2, data[1]-0x90, gReplyBuf+gReplyLen);
   gbRequest = 0xA1;		// vom RAM lesen (lassen)
   gAdr = (size_t)gReplyBuf;	// feste Speicheradresse
   if (gLen > gReplyLen) gLen = gReplyLen;
   gReplyLen = 0;		// für's nächste ProcessInOut sei gReplyBuf leer
// Cypress' EZUSB-kompatible Routinen (VendAx.hex)
  }else if (data[1] >= 0xA1 && data[1] <= 0xA3) {	// lesen
// im Sonderfall Adresse=6 und Länge=2 wird das Datums-WORD (FAT) geliefert
   if (data[1] == 0xA3 && gAdr == 6 && gLen == 2) gAdr=8;
   return 0xFF;			// usbFunctionRead() kümmert sich
  }
 }else if (data[0] == 0x40) {	// OUT, Vendor, Device
  if (data[1] >= 0x90 && data[1] <= 0x94) {
// bis zu 4 Bytes von SETUPDAT als OUT-Daten verarbeiten (Bandbreite sparen)
   gReplyLen += ProcessInOut(data+2, data[1]-0x90, gReplyBuf+gReplyLen);
   gbRequest = 0x90;	// alle weiteren OUT-Daten in usbFunctionWrite abarbeiten lassen
  }else if (data[1] == 0xA1 || data[1] == 0xA2) {	// RAM oder EEPROM schreiben
   return 0xFF;			// usbFunctionWrite() kümmert sich
  }
 }
 return 0;
}

// Datenübertragungsfunktionen für Cypress-kompatible "lange" Setup-Transfers
// sowie - neu - für ehemals Bulk-Daten
USB_PUBLIC uchar usbFunctionRead(uchar *data, uchar len) {
 void *adr = (void*)gAdr;	// im Register halten
 if (len > gLen) len = gLen;	// begrenzen auf Restlänge
 if (gbRequest == 0xA1) {	// RAM (oder speziell gReplyBuf) lesen
  memcpy(data, adr, len);
 }else if (gbRequest == 0xA2) {	// EEPROM lesen
  eeprom_read_block(data, adr, len);
 }else if (gbRequest == 0xA3) {	// Flash lesen
  memcpy_P(data, (PGM_P)adr, len);
 }else return 0xFF;		// Fehler: nichts zu liefern! (STALL EP0)
 gAdr = (size_t)adr + len;
 gLen -= len;
 return len;
}

USB_PUBLIC uchar usbFunctionWrite(uchar *data, uchar len) {
 void *adr = (void*)gAdr;	// im Register halten
 if (len > gLen) len = gLen;	// begrenzen auf Restlänge
 if (gbRequest == 0xA1) {	// RAM schreiben
  memcpy(adr, data, len);
  adr += len;
 }else if (gbRequest == 0xA2) {	// EEPROM schreiben
  uchar ll;
  for (ll = 0; ll < len; ll++) {
   eeprom_write_byte(adr, *data++);	// statt eeprom_write_block()
   wdt_reset();			// nach jedem Byte Watchdog beruhigen
   adr++;
  }
 }else if (gbRequest == 0x90) {	// OUT-Daten über EP0 (Vista/Linux-Kompatibilität)
  gReplyLen += ProcessInOut(data, len, gReplyBuf+gReplyLen);
 }else return 0xFF;		// Fehler: unerwartete Daten (STALL EP0)
 gAdr = (size_t)adr;
 gLen -= len;
 return len;
}

USB_PUBLIC void usbFunctionWriteOut(uchar *data, uchar len) {
 uchar tmp=ExpectEP1OutToken;
 if (usbCurrentDataToken==tmp) {
  ExpectEP1OutToken=tmp^USBPID_DATA0^USBPID_DATA1;
  uchar InDataLen = ProcessInOut(data, len, gReplyBuf);
  if (InDataLen) usbSetInterrupt(gReplyBuf, InDataLen);	// Antwort zum Host
 }
}

#define W(x) (x)&0xFF,(x)>>8
#define D(x) W((x)&0xFFFF),W((x)>>16)
 
#include <stdlib.h>	// wchar_t
// Die Reihenfolge der Datenablage im EEPROM ist bei gcc 3.x umgekehrt zur
// Definitionsreihenfolge, bei gcc 4.x in Definitionsreihenfolge.
// Das ist ärgerlich, so muss für etwas Ordnung alles in eine dicke struct.
EEMEM const struct{
 char usbDescriptorDeviceT[18];
 char usbDescriptorConfigurationT[32];
 wchar_t usbString0T[0x03];
 wchar_t usbStringVendorT[0x12];
 wchar_t usbStringProductEn[0x1A];
 wchar_t usbStringProductDe[0x1F];
}EE={
/*usbDescriptorDeviceT*/ {
 18,		//bLength
 USBDESCR_DEVICE,        // descriptor type	1
 W(0x0110),	// USB version supported
 -1,		// USB_CFG_DEVICE_CLASS,
 0,		// USB_CFG_DEVICE_SUBCLASS,
 0,		// protocol
 8,		// max packet size
 W(0x16c0),	// vendor ID = VOTI, Teilbereich siphec, Teilbereich h#s
 W(0x06B5),	// product ID = USB2LPT mit 1 Interface
 W(BCDDATE),	// version (ganze Monate)
 1,		// manufacturer string index
 2,		// product string index
 0,		// serial number string index		PATCH offset 16
 1},		// number of configurations
 
/*usbDescriptorConfigurationT*/ {
 9,	//bLength
 2,	//bDescriptorType	2=CONFIG
 W(32),	//wTotalLength
 1,	//bNumInterfaces
 1,	//bConfigurationValue (willkürliche Nummer dieser K.)
 0,	//iConfiguration	ohne Text
 0x80,	//bmAttributes (Busversorgt, kein Aufwecken)
 100/2,	//MaxPower (in 2 Milliampere)	100 mA
//Interface-Beschreiber 0, Alternative 0:
 9,	//bLength
 4,	//bDescriptorType	INTERFACE
 0,	//bInterfaceNumber
 0,	//bAlternateSetting
 2,	//bNumEndpoints
 -1,	//bInterfaceClass	hersteller-spezifisch
 0,	//bInterfaceSubClass	(passt in keine Klasse)
 0,	//bInterfaceProtocol
 0,	//iInterface		ohne Text
//Enden-Beschreiber C0I0A1:Interrupt EP1OUT
 7,	//bLength
 5,	//bDescriptorType	ENDPOINT
 1,	//bEndpointAddress	EP1OUT
 3,	//bmAttributes		INTERRUPT
 W(8),	//wMaxPacketSize
 10,	//bInterval		10 ms Abfrageintervall (min. zulässig)
//Enden-Beschreiber C0I0A1:Interrupt EP1IN
 7,	//bLength
 5,	//bDescriptorType	ENDPOINT
 0x81,	//bEndpointAddress	EP1IN
 3,	//bmAttributes		INTERRUPT
 W(8),	//wMaxPacketSize
 10},	//bInterval		10 ms Abfrageintervall (min. zulässig)

/*usbString0T*/		L"\x0306" L"\x0407" L"\x0409",
/*usbStringVendorT*/	L"\x0324" L"haftmann#software",
/*usbStringProductEn*/	L"\x0334" L"USB2LPT low-speed adapter",
/*usbStringProductDe*/	L"\x033E" L"USB-zu-LPT-Umsetzer, Low-Speed"
};	// Letzterer scheint erst ab Vista verwendet zu werden

// itoa() ist zu fett! Daher diese Ersatzroutine.
static uchar hexDigit(uchar nibble) {
 if (nibble>=10) nibble+=7;
 nibble+='0';
 return nibble;
}

static void BuildStringFromSerial() {
 uchar *s=0;
 uchar *d=gReplyBuf;
 *d++=18;
 *d++=USBDESCR_STRING;	//3
 do{
  uchar c=eeprom_read_byte(--s);
  *d++=hexDigit(c>>4);
  *d++=0;
  *d++=hexDigit(c&0x0F);
  *d++=0;
 }while ((uchar)(int)s!=0xFC);
}

static void loadDescriptor(const void*ee_ptr) {
 eeprom_read_block(gReplyBuf,ee_ptr,sizeof gReplyBuf);
}

USB_PUBLIC usbMsgLen_t usbFunctionDescriptor(struct usbRequest *rq) {
 uchar ret=0;
 uchar bRequest=rq->wValue.bytes[1];
 if (bRequest==USBDESCR_DEVICE) {	//1
  loadDescriptor(EE.usbDescriptorDeviceT);
  if (Feature&0x10) gReplyBuf[16]=3;
  ret=gReplyBuf[0];
 }else if (bRequest==USBDESCR_CONFIG) {	//2
  loadDescriptor(EE.usbDescriptorConfigurationT);
  if (Feature&0x80) gReplyBuf[21]=gReplyBuf[28]=2;	//BULK
  ret=sizeof EE.usbDescriptorConfigurationT;
 }else if (bRequest==USBDESCR_STRING) {	//3
  uchar wValueL=rq->wValue.bytes[0];
  if (wValueL==0) loadDescriptor(EE.usbString0T);
  else if (wValueL==1) loadDescriptor(EE.usbStringVendorT);
  else if (wValueL==2) loadDescriptor(rq->wIndex.bytes[0]==7?EE.usbStringProductDe:EE.usbStringProductEn);
  else if (wValueL==3) BuildStringFromSerial();
  ret=gReplyBuf[0];
 }
 usbMsgPtr=(usbMsgPtr_t)gReplyBuf; 
 return ret;
}


int main(void) {
// continue initialization that does not fit into interrupt vector table
 asm volatile(
"	out	0x3E,r31\n"	// SPH
"	out	0x3D,r30\n"	// SPL
"	clr	r1\n"
"1:	st	-Z,r1\n"	// clear entire BSS
"	cpi	r30,0x60\n"	// this code is ATmega8 specific
"	cpc	r31,r1\n"
"	brne	1b");
// initialize two bytes of data declared inside USBDRV.C,
// because there is no data copy routine (this here is shorter)
 usbTxLen = USBPID_NAK;	
 usbMsgLen = USB_NO_MSG; 
 ExpectEP1OutToken=USBPID_DATA0;

 uchar SofCmp = 0;
 MCUSR = 0;
 wdt_disable();		// Watchdog-Zustand überlebt Reset! Ausschalten!
 PRR = 0xEF;		// alle(!) getaktete Peripherie totlegen
 ACSR |= 0x80;		// Analogvergleicher ausschalten - 70 µA Strom sparen
 usbInit();		// Pegelwechsel-Interrupt aktivieren
 sei();
 set_sleep_mode(SLEEP_MODE_PWR_DOWN);
 sleep_enable();
 if (USBIN & USBMASK) {	// kein SE0?
  sleep_cpu();		// Ohne Oszillator schlafen bis zum Pegelwechsel
 }			// warten bis SE0
 set_sleep_mode(SLEEP_MODE_STANDBY);	// kein ...IDLE weil keine Peripherie
 wdt_enable(WDTO_15MS);	// Watchdog ist Strom sparender als ein Zeitgeber,
			// ein 3-ms-Zeitgeber wäre aber die USB-konforme Lösung
 Feature = eeprom_read_byte((uchar*)0xFFFB); // letztes Byte vor Seriennummer
 if (Feature == 0xFF) Feature = 0;	// ungebrannt als 0 annehmen
 initIOPorts();
 LptOn();
 out402(0x20);		// bidirektionalen PS/2-Modus voreinstellen
  
 for(;;) {		// Hauptschleife
  uchar t = ECR_Bits;
  if (t & 0x04) SppXfer();		// SPP-FIFO-Transfer prüfen
  else if (t & 0x08) EcpXfer();		// ECP-FIFO-Transfer prüfen
  else if (USBIN & USBMASK) sleep_cpu();// Schlafen, außer bei SE0
  t = usbSofCount;
  if (SofCmp != t || !(USBIN & USBMASK)) {
   wdt_reset();		// alle 1 ms (oder öfter bei SE0) Watchdog beruhigen
  }
  usbPoll();
  if (SofCmp != t) {	// SOF eingetroffen?
   SofCmp = t;
   Led_On1ms();		// LED blinken lassen
   if (eeprom_is_ready()	// Feature-Byte im EEPROM nachführen
   && eeprom_read_byte((uchar*)0xFFFB) != Feature)
     eeprom_write_byte((uchar*)0xFFFB, Feature);
  }
 }
}

/*** Untersuchung Stromverbrauch USB-Standby (und Normalbetrieb) ***
 * ATmega8 (ehemals):
 Wie erwartet läuft alle 15 ms der Quarzoszillator kurz an, um den Pegel
 bei ICP zu prüfen. Das erfordert ein USB-Reset (SE0) > 15 ms.
 Der Standard fordert nur 10 ms, jeder Host gibt jedoch 50 ms SE0 aus.
 Der mittlere Stromverbrauch liegt bei 500 µA, also 300 µA des ATmega8
 und 200 µA des Pullups.
 Im Normalbetrieb beträgt die Stromaufnahme datenblattgerechte 7 mA.
 * ATmega48:
 Überraschung: Die Stromaufnahme des AVR im USB-Standby beträgt ca. 300 µA -
 statt erwarteter 25 µA (für den Brown-Out-Detektor)!
 Ursache ist, dass beim mit 5 V betriebenen AVR an D- ständig 3,3 V
 anliegen, das ist alles nur wegen dieses unsauberen Pegels an einen Pin.
 Der Optimalwert für den Pullup ist hier 4,7 kOhm; dann zieht der
 AVR ca. 50 µA (bei 3,7 V), und der Pullup 250 µA.
 Noch kleinere Werte für den Pullup führen zu erhöhter Gesamtstromaufnahme
 wegen Durchbruch der Gateschutzdiode am Eingang des USB-Hostcontrollers.
 Die Probleme entfallen beim Betrieb des AVR an 3,3 V -
 hier nicht gewollt wegen erforderlicher 5-V-E/A auf LPT-Seite.
 Die Stromaufnahme des AVR im Betrieb (laufender Oszillator) beträgt 680 µA.
 Hinzu kommen 200 µA des Pullup (10 kOhm) und einige mA durch die LED.
 ERGO: Die Polling-Lösung mit dem ATmega8 war gar nicht so schlecht!
 Weil beim Tiefschlaf in jenem Fall _alle_ Portpins abgetrennt sind.
 Ein Problem des ATmega48 ist die höher liegende digitale Schaltschwelle!
*/
