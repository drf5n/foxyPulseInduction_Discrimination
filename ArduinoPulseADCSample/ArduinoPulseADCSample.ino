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
const int numSamples = 500; // sample set
// Change the sampling speed with the prescaler config in setup()


volatile uint16_t samples[numSamples];
volatile int sample = 0;
volatile bool burst = 0;
volatile bool pulse = false;
volatile int adcReading;
volatile unsigned long sampleEnd;
volatile unsigned long pulseStart, pulseEnd;

void setup ()
{
  Serial.begin (115200);
  pinMode(pulsePin, OUTPUT);

  // set the analog reference (high two bits of ADMUX) and select the
  // channel (low 4 bits).  this also sets ADLAR (left-adjust result)
  // to 0 (the default).
  ADMUX = bit (REFS0) | (adcPin & 0x07);

  // Set the ADC ADPSx prescaler flags to control sampling speed/accuracy
  ADCSRA &= ~(bit (ADPS0) | bit (ADPS1) | bit (ADPS2)); // clear prescaler bits

  //ADCSRA |= bit (ADPS0);                               //   2    5 bit, 9us
  //ADCSRA |= bit (ADPS1);                               //   4    6 bit, 9us
  //ADCSRA |= bit (ADPS0) | bit (ADPS1);                 //   8    9 bit, 9.19us
  ADCSRA |= bit (ADPS2);                                 //  16   10 bit, 13us
  //ADCSRA |= bit (ADPS0) | bit (ADPS2);                 //  32
  //ADCSRA |= bit (ADPS1) | bit (ADPS2);                 //  64
  //ADCSRA |= bit (ADPS0) | bit (ADPS1) | bit (ADPS2);   // 128

  //enable automatic conversions, start them and interrupt on finish
  ADCSRA |= bit(ADATE) | bit (ADSC) | bit (ADIE);


}  // end of setup

// ADC complete ISR
ISR (ADC_vect)
{
  unsigned long now_us;
  // Handle samples
  if ( burst ) {
    samples[sample++ ] = ADC;
    if (sample >= numSamples)
    {
      sampleEnd = micros();
      burst = false;
    }
  }
  // handle pulse end in sync with reading
  // ...
  if (pulse && ((now_us = micros()) - pulseStart >= pulse_us )) {
    *portInputRegister(digitalPinToPort(pulsePin)) = digitalPinToBitMask(pulsePin);
    pulseEnd = now_us;
    pulse = false;
  }

}  // end of ADC_vect

void loop ()
{
  char c;
  char buff[80];
  unsigned long sampleDuration;
  unsigned long pulseDuration;
  // if last reading finished, process it

  // if we aren't taking a reading, start a new one

  if (Serial.available()) {
    c = Serial.read();
    switch (c) {
      case 'c': // Converting?
        Serial.print("ADCSRA: ");
        Serial.println(ADCSRA, BIN);
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
        Serial.print((0.5 + adcReading) * 5.0 / 1024, 4);
        Serial.println(" V");
        break;
      case 'm':
        { while (burst); // don't report during burst
          unsigned long sampleDuration = sampleEnd - pulseStart;
          unsigned long pulseDuration  = pulseEnd - pulseStart;

          Serial.print("# Pulse: ");
          Serial.print(pulseDuration);
          Serial.print("us and ");
          Serial.print(numSamples);
          Serial.println(" samples.");
          Serial.print("# Time: ");
          Serial.print(sampleDuration);
          Serial.print("us burst of ");
          Serial.print(numSamples);
          Serial.print(" samples at ");
          Serial.print(1.0 * (unsigned long)(sampleEnd - pulseStart) / numSamples);
          Serial.println("us/sample");
          sprintf(buff, " sampleEnd, pulseEnd, pulseStart %02lu : %02lu : %02lu \n", sampleEnd, pulseEnd, pulseStart);
          Serial.print(buff);
          break;
        }
      case 'p': // start pulse
        sample = 0; 
        pulseStart = micros();
        //        pulseEnd = pulseStart + pulse_us;
        *portOutputRegister(digitalPinToPort(pulsePin)) |= digitalPinToBitMask(pulsePin);
        pulse = true;
        burst = true;
        break;
      case 'D':
      case 'd': {// report Data
          while (burst);
          sampleDuration = sampleEnd - pulseStart;
          for (int ii = 0 ; ii < numSamples ; ii++) {
          if (c =='D') {
            Serial.print(ii*sampleDuration/numSamples);
            Serial.print(' ');
          }
            Serial.println(samples[ii]);
          }
        }
        break;
      case ' ':
      case '\n':
      case '\r':
        break;
      case 'h':
      case '?':
      default:
        {
          Serial.println("\nArduinoPulseADCSample.in -- Pulse A1 and read a burst of samples from A0\n"
                         "based on serial commands");
          Serial.println("# https://github.com/drf5n/foxyPulseInduction_Discrimination/tree/discrimination/ArduinoPulseADCSample\nCommands: [vcsrpdm] ?");
          Serial.println("Commands:\n"
                         "p: Pulse -- Start a pulse cycle on A1 and record data on A0\n"
                         "d,D: Data -- Dump the data from the last pulse\n"
                         "m: Metadata -- Print the length of pulse, number of samples and rate\n"
                         "v: Voltage -- Conver voltage on A0\n"
                         "c: Converting? -- show ADCSSRA register\n"
                         "h?: Help -- Print this\n"
                        );
        }
        break;
        ;;
    }
  }

  // do other stuff here

}  // end of loop
