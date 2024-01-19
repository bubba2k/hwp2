// Unterste 4 bits immer send, oberste receive
// Port B0-3: empfangen
// Port C0-3: senden


uint8_t read_channel() {
  uint8_t send_bits = PINC & 0xF;
  uint8_t receive_bits = PINB & 0xF;

  return send_bits | (receive_bits >> 4);
}

void set_channel(uint8_t value) {
  uint8_t send_bits = value & 0xF;
  uint8_t receive_bits << 4;

  PORTC = send_bits;
  PORTB = receive_bits;
}

void setup() {
  DDRC = 0x0;
  DDRB = 
}

void loop() {
  // put your main code here, to run repeatedly:
}
