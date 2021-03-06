# foxyPulseInduction_Discrimination -- Pulse Induction Metal Detection tools on Arduino
 
 by  drf5n 2021-11-23 fork for DSP/curve fitting discrimination

This project was partially inpired by the Pulse Induction (PI) code by https://github.com/Dreamy16101976/foxyPIv1
but I was more interested in the digital signal processing problem of Pulse Induction. 

Many pulse induction metal detector projects choose a near-zero threshold voltage and time when the decay curve falls to that threshold.  I'm interested in the shape of the decay curve and seeing if there is interesting information in the shape to aid in discrimination.  To that end, I'm looking at how to synchonize fast ADC sampling with the pulse.

# Component project code:
* [foxyPulseInduction_Discrimination](foxyPulseInduction_Discrimination) -- fork of [https://github.com/Dreamy16101976/foxyPIv1](https://github.com/Dreamy16101976/foxyPIv1)
* [ArduinoPulseADCSample](ArduinoPulseADCSample/) -- Interactive one-shot pulse & ADC sampler. 
* [PulseGenerator](PulseGenerator/) -- Interactive pulse generator 
* [StatusCodeLED](StatusCodeLED) -- Flashes a configurable blink code on an Arduino LED 123 -> "* ** ***" 
* [DecimalFreqLF](DecimalFreqLF/) -- 20K x 0.00001Hz resolution square waves on Arduino per [https://www.romanblack.com/onesec/High_Acc_Timing.htm#decfreq](https://www.romanblack.com/onesec/High_Acc_Timing.htm#decfreq).
*  The code in the [PI_sampler_analogContinuousRead](PI_sampler_analogContinuousRead) subdirectory uses a Teensy LC to sample the ADC contemporaneously with
a triggering pulse.  The Teensy LC can easily capture a 30mS response pulse at 12-bit data at 15us sampling speed.  It
can easily go faster or longer.  This could be good for testing the pulse response.
* [SPICE](SPICE/) -- LTSpice simulation code (pulse induction circuits)


Code at [https://github.com/drf5n/foxyPulseInduction_Discrimination/tree/discrimination](https://github.com/drf5n/foxyPulseInduction_Discrimination/tree/discrimination)
Portions forked from [https://github.com/Dreamy16101976/foxyPIv1](https://github.com/Dreamy16101976/foxyPIv1)

# ToDo:

* Use high rate ADC (76.9kHz/13us) to get multiple measurements per pulse-decay cycle (per https://www.gammon.com.au/adc)

 * See [ArduinoPulseADCSample](ArduinoPulseADCSample/) for code.  Capable of 9-bit ADC at 6.5us

* Adapt a circuit

 * See the circuits at https://hackaday.io/project/179588-pulse-induction-metal-detector/log/192567-modelling-the-flip-coil I like the double-MOSFET coil isolation scheme. 

* Fit decay curve to ADC samples to find threshold, decay rate, and floor

 * With some ADC samples during the decay, one could get several metrics out of a decay curve:

 1. The time from the end of the excitation pulse to the start of the detected decay 
 1. The time constant of the decay curve
 1. Goodness-of-fit for measurements of the time constant between points on the curve.
