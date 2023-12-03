#include <iostream>
#include <vector>

// TODO: Implement await_begin_phase, check_phase,
// figure out how to extract the frame_buffer from receiver struct

// Masks for channel state    76543210
#define RECV_DATA_BIT_MASK  0b00000011
#define RECV_ACK_BIT_MASK   0b00000100
#define RECV_CLK_BIT_MASK   0b00001000

enum class ControlSeq {
  ESCAPE = 0x1,
  BEGIN  = 0x2,
  END    = 0x3
};

bool is_control_sequence(unsigned char byte) {
  return  (byte == (unsigned char) ControlSeq::ESCAPE) ||
          (byte == (unsigned char) ControlSeq::BEGIN)  ||
          (byte == (unsigned char) ControlSeq::END);
}

enum class ReceiverPhase {
  AWAIT_BEGIN,
  RECEIVE,
  CHECK,
};

class Receiver {
  std::vector<unsigned char> frame_buffer;

  ReceiverPhase phase;
  unsigned int n_bits_received;
  unsigned char byte_buffer;
  bool last_clock;
  bool ignore_next_control_sequence;

  bool frame_ready;

  // Helpers
  bool read_clock(unsigned char channel_state);
  unsigned char get_data_bits(unsigned char channel_state);


  // State machine phases
  void await_begin_phase(unsigned char channel_state);
  void receive_phase(unsigned char channel_state);
  void check_phase();

  public:
  bool frame_available();
  bool pull_frame(std::vector<unsigned char>& destination);
  void tick(unsigned char last_read);
  Receiver();
};

Receiver::Receiver() {
  phase = ReceiverPhase::AWAIT_BEGIN;
  n_bits_received = 0;
  byte_buffer = 0;
  last_clock = false;
  ignore_next_control_sequence = false;
  frame_ready = false;
}

// Read current state of the clock.
bool Receiver::read_clock(unsigned char channel_state) {
  return channel_state & RECV_CLK_BIT_MASK;
}

// Read the state of the data bits. (The two least significant bits)
unsigned char Receiver::get_data_bits(unsigned char channel_state) {
  return channel_state & RECV_DATA_BIT_MASK;
}

void Receiver::await_begin_phase(unsigned char channel_state) {
  // TODO: Receive a full byte here, check whether it is a BEGIN 
  // control sequence, if so, change the phase to RECEIVE
  channel_state = channel_state; // Silence unused parameter warning
}

void Receiver::check_phase() {
  // At this point, we have received a full frame. We shall test if 
  // it is valid, and (not) send ACK accordingly...
  bool valid = false;

  // TODO: Check whether its actually valid here. (Checksum or whatever)

  if(valid) {
    frame_ready = true;

    // TODO: Send ACK here!
  } else {
    // TODO: Do *not* send ACK here!
  
    // All has failed, wipe the frame buffer...
    frame_buffer.clear();
  }

  // 'ight, wait for the next frame to begin.
  phase = ReceiverPhase::AWAIT_BEGIN;
}

void Receiver::receive_phase(unsigned char channel_state) {
  // In this phase we receive bits until we have formed a full byte.
  // Then we check whether the byte is a control sequence and 
  // handle those accordingly... 
  // lastly we append it to our frame buffer.

  // Only do something if the clock has changed.
  // If the clock hath not changed, we shall abort this tick.
  bool new_clock = read_clock(channel_state);
  if(new_clock == last_clock) {
    return;
  } else {
    last_clock = new_clock;
  }

  // Clock has changed, let us read the new data bits and shift them into the byte.
  // First, get the two new bits.
  unsigned char new_bits = get_data_bits(channel_state);

  // Shift to make space, then set the two new bits.
  byte_buffer = byte_buffer << 2;
  byte_buffer = byte_buffer | new_bits;
  // Update how many bits of the current byte we have received.
  n_bits_received += 2;

  // If we received a full byte at this point...
  if(n_bits_received == 8) {

    // Check whether it is a control sequence... and handle it appropriately
    if(is_control_sequence(byte_buffer)) {

      // Ignore this one, but don't ignore the next one, obviously.
      if(ignore_next_control_sequence) {
        ignore_next_control_sequence = false;
      }
      else {  // Handle the escape sequences.
        switch(static_cast<ControlSeq>(byte_buffer)) {
          // We shall ignore the next control sequence ...
          case ControlSeq::ESCAPE:
            ignore_next_control_sequence = true;
            break;
          // No reason to do anything here, really ...
          // In fact, this case below should never execute!
          case ControlSeq::BEGIN:
            break;
          // Frame end... proceed with check phase...
          case ControlSeq::END:
            phase = ReceiverPhase::CHECK;
            break;
        }
      }
    }

    // Push the byte, reset bit counter.
    frame_buffer.push_back(byte_buffer);
    n_bits_received = 0;
  }
}

void Receiver::tick(unsigned char channel_state) {
  switch(phase) {
    case ReceiverPhase::RECEIVE:
      receive_phase(channel_state);
      break;
    case ReceiverPhase::AWAIT_BEGIN:
      await_begin_phase(channel_state);
      break;
    case ReceiverPhase::CHECK:
      check_phase();
      break;
  }
}

bool Receiver::frame_available() {
  return frame_ready;
}

// Copy the 
bool Receiver::pull_frame(std::vector<unsigned char>& destination) {
  if(frame_ready) {
    // Copy in to destination byte by byte ......
    destination.clear();
    for(const auto& byte : frame_buffer) {
      destination.push_back(byte);
    }

    // Wipe the frame buffer... unset ready flag...
    frame_ready = !frame_ready;
    frame_buffer.clear();
    return true;
  } 
  else {
    return false;
  }
}


int main(int argc, char *argv[]) {

  Receiver receiver;

  for(;;) {
    // unsigned char channel_state = ...read channel state here..();
    // receiver.tick(channel_state);
    // if(receiver.frame_available()) {
    //  ... collect the frame and do something with it ...
    // }
  }


  return 0;
}
