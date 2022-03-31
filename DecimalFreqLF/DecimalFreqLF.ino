// DecimalFreqLF.ino -- based on DecimalFreqLF.c from https://www.romanblack.com/onesec/High_Acc_Timing.htm#decfreq
//  DecimalFreqLF.c  Generates exact decimal frequency on a PIC using any xtal.
//  Copyright; Sep 2012 Roman Black
//  Compiler; MikroC or MikroC PRO
//  Open source; use it for whatever you like, but please mention me as the
//  original author and mention my web page; www.RomanBlack.com/High_Acc_Timing.htm
//
// Adapted to Arduino from DecimalFreqLF.c by David Forrest, 2022-01-08
// This uses the bresenham slope algorithm to keep track of the unused remainders
// between millihertz cycles and timer interrupts with the reaminder left in 'accum'
//
// Simulation https://wokwi.com/projects/327595235736552020
// github: https://github.com/drf5n/foxyPulseInduction_Discrimination/blob/discrimination/DecimalFreqLF/DecimalFreqLF.ino
//
// Sample math:
// First, set interrupts.   Choosing 50us w/100 cycles of a 2MHz clock (OCR1A=100,PS=8)
//
// For 1Hz, FreqHzD5=100000, and need to Toggle twice as fast, or 2Hz or every 500000us,
// so that means 500000us / 50us = 10000 interrupts per toggle.
// Since we accumulate the HzD5 fixed point value every interrupt, the
// for 1Hz, we add 100K/interrupt, thats 100K*10K = 1000M accumulated per toggle
// 1000M is 500x our 2MHz clock frequency.  This is the source of the '500' magic number
// It would change if interrupts were different than clock/100 or if the fixed-point format were
// different than HzD5.
//  (100000accums/interrupt/Hz)/(100clocks/interrupt)/(2 toggle/Hz) = 500

#define DEBUG 1
//
//#define HZ 16000000UL
#define HZ F_CPU

#define ck_PS 8  /* 4 for original code's PIC, 8 for TCCR1B's CS13:0 bits for /8 prescaler */
//#define TOGGLE (HZ * 125 ) /* Magic 125 from PIC's Timer /4 * 500  Must be less than 2^32 */
#define TOGGLE (500ULL * HZ / ck_PS  ) /* Magic 500 from  Must be less than 2^32 */

#define toggle_pin LED_BUILTIN

volatile unsigned long accum;    // Bresenham algorithm accumulator
volatile unsigned long freqHzD5; // fixed point with 5 decimal places
volatile unsigned long toggleCounter; //

void setup_timer1(void) {
  // for 16MHz/8 ticks and 16MHz/8/100=20000Hz or  interrupts
  // Choose WGM=15 Fast PWM TOP=OCR1A
  // Multiple WGM modes work, but each need different interrupt handling
  // WGM=15: Fast PWm with TOP=OCR1A and TIMER1_OVF_vect or TIMER1_COMPA_vect
  // WGM=14: Fast PWM with TOP=ICR1 and a TIMER1_ICF1_vect or TIMER1_OVF handler works
  // WGM=12: CTC with TOP=ICR1 and a TIMER1_ICF1_vect handler works
  // WGM=4: CTC with TOP=OCR1A and a TIMER1_COMPA_vect handler works
  // Normal modes work if you TCCR1A+=100; on each trigger
  // WGM=0: Normal mode TOP=0xFFFF works with timer TCCR1A+=100; in TIMER1_COMPA_vect
  // You might be able to do this with timer 0 in normal mode.
  // Problems:
  // WGM=5: TOP=0x00FF works with timer TCCR1A+=100; in TIMER1_COMPA_vect
  cli();
  // 16 bit Timer 1 set to trigger ever 100 ticks in WGM=15 / 0b1111, Fast PWM(OCR1A)
  TCCR1A = (0b11 << WGM10) | (0b00 << COM1A0) | (0b00 << COM1B0) ; // low WGM bits & outputs
  TCCR1B = (0b11 << WGM12) | (0b010 << CS10);   // high WGM bits & /8 prescaler
  OCR1A = (100 - 1); // set  TOP for ticks for WGM modes 4,9,11,15
  //ICR1 = (100-1);  // set  TOP for ticks for WGM modes 8,10,12,14
  // Choose interrupt to match mode
  //TIMSK1 = bit(TOIE1); // Timer Overflow Interrupt Enable 1
  TIMSK1 = bit(OCIE1A); // Output Compare Interrupt Enable 1 (needs TIMER1_COMPA_vect)
  //TIMSK1 = bit(ICF1) ; // Input Capture Flag 1 (Needs TIMER1_CAPT_vect)
  sei();
}


//ISR(TIMER1_CAPT_vect) // WGM modes with ICR1 as top (8,10,12,14)
ISR(TIMER1_COMPA_vect) // WGM modes with OCR1A as top (4,9,11,15)
//ISR(TIMER1_OVF_vect) // WGM modes which overflow at top: 8,9,10,11,14,15
{ // this counts up by Hz *10000 per interrupt, and toggles down every TOGGLE
  // 16 bit Timer 1 overflow interrupt, set to trigger every 100 ticks
  accum += freqHzD5; //
  //OCR1A += 100U; // Add 100 in WGM=0 normal mode--Not 5,6,7 w double buffering
  //TCNT1 -=99; // Almost works for WGM 5,6,7 -- not reliable
  if (accum >= TOGGLE)
  {
    //digitalWrite(toggle_pin,~digitalRead(toggle_pin));
    // or IO magic per
    *portInputRegister(digitalPinToPort(toggle_pin)) = digitalPinToBitMask(toggle_pin);
    accum -= TOGGLE;
#ifdef DEBUG
    toggleCounter++;
#endif
  }
}

// Duplicate ISR calls per https://www.nongnu.org/avr-libc/user-manual/group__avr__interrupts.html
ISR(TIMER1_CAPT_vect, ISR_ALIASOF(TIMER1_COMPA_vect));
ISR(TIMER1_OVF_vect, ISR_ALIASOF(TIMER1_COMPA_vect));


void setup() {
  char buff[100];
  Serial.begin(115200);
  pinMode(toggle_pin, OUTPUT);
  digitalWrite(toggle_pin, 0);
  setup_timer1();
  freqHzD5 = 987565432UL; // Fixed-point value for Hertz with 5 decimal places at 0.00001Hz resolution
  accum = 0;

  sprintf(buff, "%lu.%05lu", freqHzD5 / 100000, freqHzD5 % 100000UL);
  Serial.print("Cycling Pin 13 at ");
  Serial.print(buff);
  Serial.print("Hz using a Bresenham algorithm with TOGGLE = ");
  sprintf(buff, "%lu%06luULL", TOGGLE / 1000000, TOGGLE % 1000000);
  Serial.print(buff);
  Serial.print(" & steps of freqHzD5 = ");
  Serial.print(freqHzD5);
  Serial.print("ULL.\n");
#ifdef DEBUG
  Serial.print("DEBUG output: \n");
#endif

}

void loop() {
  // put your main code here, to run repeatedly:

#ifdef  DEBUG
  static byte last_state = 0;
  static unsigned long last_change = 0 ;
  static unsigned long last_report = 0;
  unsigned long interval = 1000000UL;
  unsigned long duration, now ;
  char buff[180];

  now = micros();
  if (now - last_report >= interval) {
    last_report += interval;
    // while(digitalRead(toggle_pin) == last_state) ;  // delay
    if (digitalRead(toggle_pin) != last_state) {
      last_state = digitalRead(13);
      duration = now - last_change;
      last_change = now;
    }
    noInterrupts();
    unsigned long fcSave = toggleCounter;
    toggleCounter = 0;
    interrupts();
    //*portInputRegister(digitalPinToPort(toggle_pin)) = digitalPinToBitMask(toggle_pin);
    //delay(100);
    Serial.print(now);
    Serial.print("us, ");
    sprintf(buff, "%lu HzD5 in, %lu Hz out, %lu steps", freqHzD5, fcSave / 2, TOGGLE / freqHzD5);
    Serial.print(buff);
    Serial.print(" in ");
    sprintf(buff, "%lu", TOGGLE);
    Serial.print(buff);
    //Serial.print((unsigned long)TOGGLE);
    Serial.print(" toggle, ");
    Serial.print(duration);
    Serial.print("ms, ");
    Serial.print(accum);
    Serial.print(" accum, LED:");
    Serial.print(last_state);
    Serial.print(' ');

    Serial.print(" (TIMER1: TCCR1A 0b");
    Serial.print(TCCR1A, BIN);
    Serial.print(" TCCR1B 0b");
    Serial.print(TCCR1B, BIN);
    Serial.print(" OCR1A ");
    Serial.print(OCR1A);
    Serial.print(" TCNT1 ");
    Serial.print(TCNT1);
    Serial.print(" TIMSK1 0b");
    Serial.print(TIMSK1, BIN);
    Serial.print(")");
    Serial.println();
  }
#endif

}

