//: ArduinoPulseADCSample.ino -- Pulse a pin and sample response on ADC 
// For Arduino Uno/Nano/mega
// https://github.com/drf5n/foxyPulseInduction_Discrimination/tree/discrimination/ArduinoPulseADCSample
// Adapted from  http://www.gammon.com.au/forum/?id=11488&reply=5#reply5
//          and https://github.com/pedvide/ADC/tree/master/examples/analogContinuousRead
//
// This code emits a digital pulse on A1 and collects a sample of highspeed ADC from A0
// It sets up a free-running ADC to quickly record a set of ADC data in 
// response to the pulse.  The size of the pulse, pins, number of samples,
// speed of sampling, etc... are all configurable.
// The ADC interrupt handler controls the pulse length and filling the sample buffer
//
// Serial Commands:
//      
//  *  Write v and press enter on the serial console to get the value
//  *  Write c and press enter on the serial console to check that the conversion status,
//  *  Write s to stop the conversions, you can restart it writing r.
//  *  Write r to restart the conversions.
//  *  Write p to emit a pulse and record a burst of values
//  *  Write d to see the data recorded during the burst (compatible with SerialPlotter)
//  *  Write m to see metadata about the sample size and interval
//
//  Loopback test with a jumper or RC network between A1 and A2.
// 

// Configurables:
const byte adcPin = 0; // A0 -- Pin to read ADC data from
const byte pulsePin = A1; // Next to A0 -- pin to pulse
const int pulse_us = 1000; // pulse duration
const int numSamples = 2000; // sample set
// Change the sampling speed with the prescaler config in setup() 


volatile uint16_t samples[numSamples]; 
volatile int sample = 0;
volatile bool burst = 0;
volatile int adcReading;
volatile long sampleEnd;
unsigned long pulseStart, pulseEnd;

void setup ()
{
  Serial.begin (115200);
  pinMode(pulsePin,OUTPUT);

  // set the analog reference (high two bits of ADMUX) and select the
  // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
  // to 0 (the default).
  ADMUX = bit (REFS0) | (adcPin & 0x07);

  // Set the ADC ADPSx prescaler flags to control sampling speed/accuracy
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits

  //ADCSRA |= bit (ADPS0);                               //   2  
  //ADCSRA |= bit (ADPS1);                               //   4  
  //ADCSRA |= bit (ADPS0) | bit (ADPS1);                 //   8  
  ADCSRA |= bit (ADPS2);                               //  16 
  //ADCSRA |= bit (ADPS0) | bit (ADPS2);                 //  32 
  //ADCSRA |= bit (ADPS1) | bit (ADPS2);                 //  64 
  //ADCSRA |= bit (ADPS0) | bit (ADPS1) | bit (ADPS2);   // 128
  
  //enable automatic conversions, start them and interrupt on finish
  ADCSRA |= bit(ADATE) | bit (ADSC) | bit (ADIE); 
  
  Serial.println("# https://github.com/drf5n/foxyPulseInduction_Discrimination/tree/discrimination/ArduinoPulseADCSample\nCommands: [vcsrpdm] ?");
  
}  // end of setup

// ADC complete ISR
ISR (ADC_vect)
  {
  byte low, high;
  // we have to read ADCL first; doing so locks both ADCL
  // and ADCH until ADCH is read.  reading ADCL second would
  // cause the results of each conversion to be discarded,
  // as ADCL and ADCH would be locked when it completed.
  low = ADCL;
  high = ADCH;
  adcReading = (high << 8) | low;
  // handle pulse in sync with reading
  // ...
    if(*portOutputRegister(digitalPinToPort(pulsePin)) & digitalPinToBitMask(pulsePin)  && micros() >= pulseEnd){
      *portOutputRegister(digitalPinToPort(pulsePin)) &= ~digitalPinToBitMask(pulsePin);
    }

  // Handle samples
  if(burst && samples >= 0)
      {
      samples[sample--] = adcReading;
      if (sample < 0)
        {
        sampleEnd = micros();
        burst = 0;
        }
      }
  }  // end of ADC_vect
  
void loop ()
{
  char c;
  // if last reading finished, process it
    
  // if we aren't taking a reading, start a new one
 
    if (Serial.available()) {
        c = Serial.read();
        switch(c) {
          case 'c': // Converting?
             Serial.print("ADCSRA: ");
             Serial.println(ADCSRA,BIN);
             break;
          case 's': // stop conversions
            ADCSRA &= ~(bit (ADSC) | bit (ADIE));
            break;
          case 'r': // restart conversions
            ADCSRA |= bit (ADSC) | bit (ADIE);
            break;
          case 'v':
             Serial.print(adcReading);
             Serial.print(' ');
             Serial.print((0.5+adcReading)*5.0/1024,4);
             Serial.println(" V");
             break;
          case 'm':
            Serial.print("Pulse ");
            Serial.print(pulse_us);
            Serial.print("us and ");
            Serial.print(numSamples);
            Serial.println("samples. Enter 'd' for data.");
            Serial.print("# ");
            Serial.print(sampleEnd-pulseStart);
            Serial.print("us burst of ");
            Serial.println(numSamples);
            break;
          case 'p': // start pulse
            pulseStart = micros();
            pulseEnd = pulseStart+pulse_us;
            *portOutputRegister(digitalPinToPort(pulsePin)) |= digitalPinToBitMask(pulsePin);
            sample = numSamples-1;
            burst = 1;
            break;
          case 'd':// report Data
            for(int ii = 0 ; ii < numSamples ; ii++){
              Serial.println(samples[numSamples-1-ii]);
            }
            break;
          default:
           ;;
        }
     }

  // do other stuff here

}  // end of loop
