#include <vector>

#define DATA_BIT_MASK 0b0011
#define ACK_BIT_MASK  0b0100
#define CLK_BIT_MASK  0b1000

enum class ControlSeq {
  ESCAPE = 0x1,
  BEGIN  = 0x2,
  END    = 0x3
};

enum class SenderPhase {
  SEND,
  AWAIT_ACK,
};

class Sender {
  std::vector<unsigned char> byte_buffer;
  unsigned char bit_buffer;

  SenderPhase phase;

  unsigned int n_ticks_in_ack_wait;
  unsigned int n_bits_sent, n_bytes_sent;
  bool last_clock;

  bool _need_frame;

  unsigned char send_phase(unsigned char channel_state);
  unsigned char await_ack_phase(unsigned char channel_state);

  public:
  unsigned char tick(unsigned char last_read);
  bool need_frame();
  bool read_frame(const std::vector<unsigned char>& frame);
  Sender();
};

bool Sender::need_frame() {
  return _need_frame;
}

Sender::Sender() {
  bit_buffer = 0;
  phase = SenderPhase::SEND;
  n_bits_sent = 0; // Set this to 8 here so sender pulls new byte into bit buffer
                   // upon very first tick
  n_bytes_sent = 0;
  n_ticks_in_ack_wait = 0;
  last_clock = false;
  _need_frame = false;
}

bool Sender::read_frame(const std::vector<unsigned char>& frame) {
  // Do not read in a new frame if we do not need one yet.
  if(!need_frame()) return false;

  // Copy byte by byte
  byte_buffer.clear();
  for(const auto& byte : frame) {
    byte_buffer.push_back(byte);
  }

  _need_frame = false;

  return true;
}

unsigned char Sender::await_ack_phase(unsigned char channel_state) {
  // Check whether we receive an ACK (in the span of n ticks).
  // If so, request the next frame. If not, resend the current frame.
  n_ticks_in_ack_wait += 1;

  if(channel_state & ACK_BIT_MASK) { // ACK received
    // Request a new frame and reset byte counter
    _need_frame = true;
    n_bytes_sent = 0;

    // Now just go back to SEND phase.
    phase = SenderPhase::SEND;
    n_ticks_in_ack_wait = 0;
  }
  else if(n_ticks_in_ack_wait > 20) { // Counting ticks is a cheap hack...
    // Seems like we are not getting our ACK... Resend the current frame.
    _need_frame = false;  // Stays false.
    n_bytes_sent = 0;     // Reset byte counter.

    phase = SenderPhase::SEND;
    n_ticks_in_ack_wait = 0;
  }

  return channel_state;
}

// All we do here is just send all the data in the byte buffer 2 bits at a time.
// Easy enough.
unsigned char Sender::send_phase(unsigned char channel_state) {
  // Gonna need this later on ...
  unsigned char new_channel_state;

  // Get the new clock value (just flip it)
  bool new_clock = !last_clock;
  last_clock = new_clock;

  // Check whether we need to copy a new byte to our bit buffer
  if(n_bits_sent == 8) {
    bit_buffer = byte_buffer[n_bytes_sent++];
  }

  // Get the data bits
  unsigned char data_bits = bit_buffer & DATA_BIT_MASK;
  bit_buffer = bit_buffer >> 2; // Shift so the next 2 bits become
                                // available next tick.
  n_bits_sent += 2;

  // Assemble the channel state we wish to post.
  // Make sure to leave the ACK bit unaffected.
  unsigned char clock_bits = new_clock ? (0x1 << 4) : 0x0;
  new_channel_state = channel_state & 0b0100; // Clear all bits except
                                              // the ACK bits
  // Set the clock and data bits.
  new_channel_state |= clock_bits;
  new_channel_state |= data_bits;

  // Check whether the frame is done. If so, simply proceed to AWAIT_ACK phase.
  // The AWAIT_ACK phase will take care of requesting a new buffer etc.
  if(n_bytes_sent == byte_buffer.size()) {
    phase = SenderPhase::AWAIT_ACK;
  }

  // Always return the channel state to be posted.
  return new_channel_state;
}

unsigned char Sender::tick(unsigned char channel_state) {
  switch(phase) {
    case SenderPhase::SEND:
      return send_phase(channel_state);
      break;
    case SenderPhase::AWAIT_ACK:
      return await_ack_phase(channel_state);
      break;
    default:  // This case should NEVER happen.
      return 0x0;
      break;
  }
}

int main() {

  /*
  Sender sender;

  while(true) {
    if(sender.need_frame())
      sender.read_frame() ... read in a new frame here
  }
  */

  return 0;
}
