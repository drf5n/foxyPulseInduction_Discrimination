# foxyPulseInduction_Discrimination 
 by  drf5n 2021-11-23 fork for DSP/curve fitting discrimination

This project was partially inpired by the Pulse Induction (PI) code by https://github.com/Dreamy16101976/foxyPIv1
but I was more interested in the digital signal processing problem of PI.

The code in the  PI_sampler_analogContinuousRead subdirectory uses a Teensy LC to sample the ADC contemporaneosly with
a triggering pulse.  The Teensy LC can easily capture a 30mS response pulse at 12-bit data at 15us sampling speed.  It
can easily go faster or longer.  This would be good for testing the pulse response.

  

Code at https://github.com/drf5n/foxyPulseInduction_Discrimination/tree/discrimination 
Portions forked from https://github.com/Dreamy16101976/foxyPIv1

# ToDo:

* Use high rate ADC (76.9kHz/13us) to get multiple measurements per pulse-decay cycle (per https://www.gammon.com.au/adc)
* Adapt a circuit
* Fit decay curve to ADC samples to find threshold, decay rate, and floor

