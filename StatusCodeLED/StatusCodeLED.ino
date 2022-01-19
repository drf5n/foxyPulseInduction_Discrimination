// StatusCodeLED.ino 
// Produce a 0-255 background status/heartbeat code on LED_BUILTIN
// Inspired by https://forum.arduino.cc/t/status-led-function-needed/119372
// D. Forrest, (C) 2022-01-18, CC BY-SA/GPL-3

//############### StatusCodeLED 
// global variables
uint8_t statusCode = 123;  // Global variable to be pulsed on the LED
uint32_t statusCodeBitcode = 0; // statusCode 123 -> 0b11101101000

uint32_t status_code_to_bitstream(byte code) {
  // translate a decimal code into a R->L stream of bits for pulsning
  //  Example: 145 -> 0b111110111101000
  uint32_t bits = 0;
  short digit = 0;
  while (code) {
    digit = code % 10;
    code = code / 10;
    for ( ; digit > 0; digit--) {
      bits = (bits << 1) | 0b1;
    }
    bits <<= 1; // Inter-digit pause
  }
  bits <<= 2; // Longer pause at beginning
  return bits;
}

void setupStatusCodeLED(int code) {
  // Co-opt Timer0's config with an extra 16MHz/1024 interrupt:
  statusCode = code;
  statusCodeBitcode = status_code_to_bitstream(statusCode);
  OCR0A = 0x8f; // halfway to overflow
  TIMSK0 |= bit(OCIE0A); // enable TIMER0_COMPA_vect
  pinMode(LED_BUILTIN, OUTPUT);
}

ISR (TIMER0_COMPA_vect) {  // Co-opt of TIMER1 per setupStatusCodeLED(code)
  StatusCodeLED();
}

void StatusCodeLED(void) {
  // Flash LED based on statusCode / statusCodeBitmap
  static int code_ms = 0; // SV for time in cycle
  if (code_ms++ & 0xff ) return; // return control most of the time
  digitalWrite(LED_BUILTIN,
               ((statusCodeBitcode >> (code_ms >> 9)) & 0b1) // half-second per bit
               && (code_ms & 0x100 )); // toggle 1/4sec
  if ((statusCodeBitcode >> (code_ms >> 9 ) == 0)) code_ms = 0; //reset cycle
  // Debugging:
  if (0 & code_ms % 1024 == 0 && statusCodeBitcode) {
    Serial.print(" statusCode: ");
    Serial.print((int)statusCode, DEC);
    Serial.print(" 0b");
    Serial.println(statusCodeBitcode, BIN);
  }
}
// ########  StatusCodeLED END

// ####### sample sketch and testing:
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setupStatusCodeLED(123); // Heartbeat "*-**-***" on LED_BUILTIN
}

void loop() {
  // put your main code here, to run repeatedly:

  if (1) { //#### Status Code Heartbeat Debugging and test pattern
    delay(60000); // 60 sec default, then throw random sequence
    for (int ii = 1 ; ii  ; ii = (ii * 17 + 1) % 256) {
      // Run a cycle of an 8 bit  LCG X=17*X+1
      setupStatusCodeLED(ii);

      if (Serial) {
        Serial.print("StatusCodeLED(");
        Serial.print(ii);
        Serial.print(") = 0b");
        Serial.println(statusCodeBitcode, BIN );
      }
      delay(30000); //
    }
    setupStatusCodeLED(119);
  }
}
