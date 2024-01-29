// Unterste 4 bits immer send, oberste receive
// Port C0-3: empfangen (Arduino A0-A3)
// Port B0-3: senden   (Arduino D8-D11)

uint8_t read_channel() {
  uint8_t send_bits = PINB & 0xF;
  uint8_t receive_bits = PINC & 0xF;

  return send_bits | (receive_bits << 4);
}

void set_channel_split(uint8_t send_bits, uint8_t receive_bits) {
  PORTB = send_bits & 0xF;
  PORTC = receive_bits & 0xF;
}

void set_channel(uint8_t value) {
  uint8_t send_bits = value & 0xF;
  uint8_t receive_bits = value << 4;

  PORTC = receive_bits;
  PORTB  = send_bits;
}

void setup() {
  // Receive
  DDRC = 0b0100;
  // Send
  DDRB = 0b1011;


  Serial.begin(9600);

}

static volatile unsigned char i = 0;

void loop() {
  // Read the channel state and send it to serial.
  uint8_t current_channel_state = read_channel();
  Serial.write(current_channel_state);
  
  // Read the requested channel state from serial and set it.
  uint8_t requested_channel_state = Serial.read();
  set_channel(requested_channel_state);
}
