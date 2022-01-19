# ArduinoPulseADCSample.ino -- Use an Arduino to create a pulse and ADC sample the result
 by drf5n 2021-12-08

This project was partially inpired by the Pulse Induction (PI) code by https://github.com/Dreamy16101976/foxyPIv1
but I was more interested in the digital signal processing problem of PI.

The code in the  PI_sampler_analogContinuousRead subdirectory uses a Teensy LC to sample the ADC contemporaneosly with
a triggering pulse.  The Teensy LC can easily capture a 30mS response pulse at 12-bit data at 15us sampling speed.  It
can easily go faster or longer.  This would be good for testing the pulse response.


    
This is the Serial Monitoroutput from the p,m,d commands with a jumper connecting pin 12 to A0 with the /16 prescaler:
    
    # Pulse: 50us and 20 samples.
    # Time: 276us burst of 20 samples at 13.80us/sample
     sampleEnd, pulseEnd, pulseStart 19270968 : 19270742 : 19270692 
    0
    1020
    1023
    1023
    1023
    3
    0
    0
    0
    0
    0
    0
    0
    0
    0
    0
    0
    0
    0
    0
    
 
Here's an R-C pulse-decay trace on the Serial Plotter with a 22K resistor across A0&12 and a 22uF capacitor from GND to A0:


 [R-C pulse plot](https://i.stack.imgur.com/ztEva.png)

 [![R-C pulse plot](https://i.stack.imgur.com/ztEva.png)](https://arduino.stackexchange.com/questions/86808/arduino-fast-adc-sampling-which-burst-control-is-best/86945#86945) 
