// Example of receiving numbers by Serial
// Author: Nick Gammon
// Date: 31 March 2012

const char startOfNumberDelimiter = '<';
const char endOfNumberDelimiter   = '>';

const int prescales[] = {0,1,8,64,256,1024}; 

void help()
  {
    Serial.println("PulseGenerator: Creates variable pulses on pins 11&12 an Arduino Mega");
    Serial.println("Commands: 0123456789kmpfrd%h");
    Serial.println("For example: for 2000Hz with 20% duty cycle on and a 30 tick pulse");
    Serial.println ("Starting with 1KHz on pulse & square on D11/OC1A/PB5 & D12/OC1B/PB6\n");
    Serial.print("Send '2000f 20% 30d rh'\n");
    Serial.print("edge cases are not tested\n\n");
    Serial.println("Code at https://github.com/drf5n/foxyPulseInduction_Discrimination/blob/discrimination/PulseGenerator/PulseGenerator.ino");
  }

void setup ()
  { 
  pinMode(11,OUTPUT);
  pinMode(12,OUTPUT);
  Serial.begin (115200);
  Serial.println ("Starting with 1KHz on pulse & square on D11/OC1A/PB5 & D12/OC1B/PB6\n");
  setFrequency(1000); //1 khz
  help();
  } // end of setup
  
void processNumber (const long n)
  {
  Serial.println (n);
  }  // end of processNumber

void setFrequency (const unsigned long long freq)
  {
    Serial.print("(f)requency set to approximately ");
    Serial.print((unsigned long) freq);
    Serial.print(" Hz by setting (period) to...");
    Serial.println();
    setPeriod((unsigned long long)(1.0*F_CPU/freq));  
    ;
  }

void setPeriod (const unsigned long long period) // 
  {
    Serial.print("(p)eriod set to ");
    Serial.print((unsigned long) period);
    Serial.print(" ticks of a ");
    Serial.print(F_CPU);
    Serial.print(" Hz CPU clock\n");
    Serial.println();
    // Period 0  -2^16 PS=/1  
    // Period 2^3-2^19 PS=/8    
    // Period 2^6-2^22 PS=/64 
    // Period 2^10-2^24 PS=/256 
    // Period 2^16-2^30 PS=/1024 
    byte tccr1a = (0b10 << WGM10) // Start Timer 1 in WGM mode 14: Fast PWM mode TOP=ICR1
                  | (0B10 <<COM1A0)    //  OC1A output set->clear
                  | (0b10 << COM1B0) ; //  OC1B output set->clear;
    byte ocr1a = 1;              
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    if (period < (1ULL<<16)) { 
      Serial.print("Trying /1 prescaler with period=");
      Serial.println((unsigned long)period);
      ICR1 = (period);
      OCR1A = ocr1a;
      OCR1B = (period >>1L);
      TCCR1A = tccr1a;
      TCCR1B = (0b11 << WGM12) | (0b001 << CS10); // Start in WGM mode 14: Fast PWM, 1:1 prescaler
    } 
    else if (period < prescales[2]*65536) 
    {
      Serial.print("Trying /8 prescaler with period=");
      Serial.print((unsigned long)period,HEX);
      Serial.print(" ");
      Serial.println((unsigned long)period/prescales[2],HEX);
      ICR1 = (period /prescales[2]);
      OCR1A = ocr1a;
      OCR1B = ICR1 >>1;
      TCCR1A = tccr1a;
      TCCR1B = (0b11 << WGM12) | (0b010 << CS10); // Start in WGM mode 14: Fast PWM, 1:8 prescaler
    }
    else if (period < prescales[3]*65536 ) 
    {
      Serial.print("Trying /64 prescaler ");
      Serial.print((unsigned long)period,HEX);
      Serial.print(" ");
      Serial.println((unsigned long)period/prescales[3],HEX);
      ICR1 = (period /prescales[3]);
      OCR1A = 10;
      OCR1B = ICR1>>2;
      TCCR1A = tccr1a;
      TCCR1B = (0b11 << WGM12) | (0b011 << CS10); // Start in WGM mode 14: Fast PWM, 1:64 prescaler
    }
    else if (period < prescales[4]*65536 ) 
    {
      Serial.print("Trying /256 prescaler 0x");
      Serial.print((unsigned long)period,HEX);
      Serial.print(" 0x");
      Serial.println((unsigned long)period/prescales[4],HEX);
      ICR1 = (period /prescales[4]);
      OCR1A = ocr1a;
      OCR1B = ICR1>>2L;
      TCCR1A = tccr1a;
      TCCR1B = (0b11 << WGM12) | (0b100 << CS10); // Start in WGM mode 14: Fast PWM, 1:256 prescaler
    }
    else if (period < prescales[5]*65536 ) 
    {
      ICR1 = (period /prescales[5]);
      OCR1A = ocr1a;
      OCR1B = ICR1>>2;
      TCCR1A = tccr1a;
      TCCR1B = (0b11 << WGM12) | (0b101 << CS10); // Start in WGM mode 14: Fast PWM, 1:1024 prescaler
    }
    else 
    {
      Serial.print(" -- Period too long.  Defaulting to maximum\n0x");
      Serial.print((unsigned long)period/prescales[5],HEX);
      Serial.print(" 0x");
      Serial.println((unsigned long)period & 0xffff,HEX);
      ICR1 = 65535;
      OCR1A = ocr1a;
      OCR1B = ICR1>>2;
      TCCR1A = tccr1a;
      TCCR1B = (0b11 << WGM12) | (0b101 << CS10); // Start in WGM mode 14: Fast PWM, 1:1024 prescaler
    }
 
     
    ;
  }

void setPulse(const unsigned long long duration) // Set the pulse duration
  {
    Serial.print("(d)uration of pulse len on OC1A set to ");
    Serial.print((unsigned long) duration);
    Serial.print(" ticks\n");
    OCR1A = duration;
  }

void setDuty(const unsigned long long duration) // Set the duty cycle 
  {
    Serial.print("(%) Duty Cycle on OC1B set to ");
    Serial.print((unsigned long) duration);
    Serial.print("% of ");
    Serial.print(ICR1);
    Serial.print(" tick = ");
    Serial.print(ICR1*(unsigned long)duration/100);
    Serial.print("\n");
    OCR1B = ICR1*duration/100;
  }



void report (void)
  {
    Serial.print("(r)eport: ");
    Serial.print("");
    Serial.println("Timer 1 Configuration");
    Serial.println("TCCR1A TCCR1B ICR1 TCNT1  WGM13:0 OCR1A OCR1B ");
    Serial.print(TCCR1A,BIN);
    Serial.print(" ");
    Serial.print(TCCR1B,BIN);
    Serial.print(" ");
    Serial.print(ICR1);
    Serial.print(" ");
    Serial.print(TCNT1);
    Serial.print(" ");
    Serial.print((((TCCR1B & (0b11<<WGM12) )>>WGM12)<<2) | (TCCR1B & (0b11<<WGM10)>>WGM10),BIN );
    Serial.print(" ");
    Serial.print(OCR1A);
    Serial.print(" ");
    Serial.print(OCR1B);
    
    Serial.println(" ");
    Serial.print("Timer Clock configuration\nF_CPU   /  prescaler / ICR1 = FREQ\n");
    Serial.print(F_CPU);
    Serial.print("/");
    float scale = prescales[(TCCR1B & (0b111 << CS10))];
  //  Serial.println(TCCR1B & (0b111 << CS10),BIN);
  //  Serial.print(" / ");
    Serial.print(scale);
    Serial.print(" / ");
    Serial.print(ICR1);
    Serial.print(" = ");
    Serial.print(F_CPU/scale/ICR1);
    Serial.print("Hz, ");
    Serial.print(1.0e6 * OCR1A /(F_CPU/scale));
    Serial.println("us pulse"); 

    
    ;
  }

 
void processInput ()
  {
  static unsigned long long receivedNumber = 0;
  static boolean negative = false;
  
  byte c = Serial.read ();
  
  switch (c)
    {
      
    case endOfNumberDelimiter:  
      if (negative) 
        processNumber (- receivedNumber); 
      else
        processNumber (receivedNumber); 

    // fall through to start a new number
    case startOfNumberDelimiter: 
      receivedNumber = 0; 
      negative = false;
      break;
      
    case '0' ... '9': 
      receivedNumber *= 10;
      receivedNumber += c - '0';
      break;
      
    case '-':
      //negative = true;
      Serial.println("Negatives not supported");
      break;
    case 'k':
    case 'K':
      receivedNumber *= 1000;
      break;
    case 'm':
    case 'M':
       receivedNumber *= 1000000;
       break; 
    case 'f': // frequency
       setFrequency(receivedNumber);
       receivedNumber = 0;
       negative = false;
       break;
    case 'p': // period
       setPeriod(receivedNumber);
       receivedNumber = 0;
       negative = false;
       break;
    case 'd': // duration
       setPulse(receivedNumber);
       receivedNumber=0;
       break;
    case '%':
       setDuty(receivedNumber);
       receivedNumber=0;
       break;
     case 'h':
       help();
       break;
    case 'r': // report
       report();
       break;
    } // end of switch  
  }  // end of processInput
  
void loop ()
  {
  
  while (Serial.available ())
    processInput ();
    
  // do other stuff here
  } // end of loop
