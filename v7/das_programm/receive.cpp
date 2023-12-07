#include <iostream>
#include <vector>

#include "receive.hpp"

// TODO: Implement await_begin_phase, check_phase,
// figure out how to extract the byte_buffer from receiver struct

// Masks for channel state    76543210
#define RECV_DATA_BIT_MASK  0b00000011
#define RECV_ACK_BIT_MASK   0b00000100
#define RECV_CLK_BIT_MASK   0b00001000

// BEGIN | CHECKSUM | DATA... | END

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

Receiver::Receiver() {
  phase = ReceiverPhase::AWAIT_BEGIN;
  n_bits_received = 0;
  bit_buffer = 0;
  last_clock = false;
  ignore_next_control_sequence = false;
  frame_ready = false;
  n_ticks_ack_sent = 0;
}

// Read current state of the clock.
bool Receiver::read_clock(unsigned char channel_state) {
  return channel_state & RECV_CLK_BIT_MASK;
}

// Read the state of the data bits. (The two least significant bits)
unsigned char Receiver::get_data_bits(unsigned char channel_state) {
  return channel_state & RECV_DATA_BIT_MASK;
}

unsigned char Receiver::await_begin_phase(unsigned char channel_state) {
  // TODO: Receive a full byte here, check whether it is a BEGIN 
  // control sequence, if so, change the phase to RECEIVE

  // Only do something if the clock has changed.
  // If the clock hath not changed, we shall abort this tick.
  bool new_clock = read_clock(channel_state);
  if(new_clock == last_clock) {
    return channel_state;
  } else {
    last_clock = new_clock;
  }

  // Clock has changed, let us read the new data bits and shift them into the byte.
  // First, get the two new bits.
  unsigned char new_bits = get_data_bits(channel_state);

  // Shift to make space, then set the two new bits.
  bit_buffer = bit_buffer << 2;
  bit_buffer = bit_buffer | new_bits;
  // Update how many bits of the current byte we have received.
  n_bits_received += 2;

  // If we received a full byte at this point, check whether it is 
  // a BEGIN, if so, move on to RECEIVE phase.
  if(n_bits_received == 8) {
    if(bit_buffer == (unsigned char) ControlSeq::BEGIN) {
      phase = ReceiverPhase::RECEIVE;
    }

    n_bits_received = 0;
  }

  return channel_state;
}

unsigned char Receiver::fetch_checksum_phase(unsigned char channel_state) {
  // TODO: Implement this
  return channel_state;
}

unsigned char Receiver::check_phase(unsigned char channel_state) {
  // TODO: For testing purposes, we simply skip this phase for now
  // and always send ACK, as if the check was successful.
  phase = ReceiverPhase::SEND_ACK;
  return channel_state;



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
    byte_buffer.clear();
  }

  // 'ight, wait for the next frame to begin.
  phase = ReceiverPhase::AWAIT_BEGIN;
}

unsigned char Receiver::send_ack_phase(unsigned char channel_state) {
  // Send ACK for a few ticks here
  unsigned char new_channel_state = channel_state;

  if(n_ticks_ack_sent > 20) {
    // Done sending ACK, go back to AWAIT_BEGIN phase.
    new_channel_state &= 0b1011;  // Do not send ACK no more
    n_ticks_ack_sent = 0;
  }
  else {
    // Send out ACK
    new_channel_state |= 0b0100;
    n_ticks_ack_sent += 1;
  }

  return new_channel_state;
}

unsigned char Receiver::receive_phase(unsigned char channel_state) {
  // In this phase we receive bits until we have formed a full byte.
  // Then we check whether the byte is a control sequence and 
  // handle those accordingly... 
  // lastly we append it to our frame buffer.

  // Only do something if the clock has changed.
  // If the clock hath not changed, we shall abort this tick.
  bool new_clock = read_clock(channel_state);
  if(new_clock == last_clock) {
    return channel_state;
  } else {
    last_clock = new_clock;
  }

  // Clock has changed, let us read the new data bits and shift them into the byte.
  // First, get the two new bits.
  unsigned char new_bits = get_data_bits(channel_state);

  // Shift to make space, then set the two new bits.
  bit_buffer = bit_buffer << 2;
  bit_buffer = bit_buffer | new_bits;
  // Update how many bits of the current byte we have received.
  n_bits_received += 2;

  // If we received a full byte at this point...
  if(n_bits_received == 8) {
    // Check whether it is a control sequence... and handle it appropriately
    if(is_control_sequence(bit_buffer)) {

      // Ignore this one, but don't ignore the next one, obviously.
      if(ignore_next_control_sequence) {
        ignore_next_control_sequence = false;
      }
      else {  // Handle the escape sequences.
        switch(static_cast<ControlSeq>(bit_buffer)) {
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
            // TODO: For testing purposes, we skip the check phase and always send out
            // ACK right away here.
            phase = ReceiverPhase::SEND_ACK;
            break;
        }
      }
    }

    // Push the byte, reset bit counter.
    byte_buffer.push_back(bit_buffer);
    n_bits_received = 0;
  }

  return channel_state;
}

unsigned char Receiver::tick(unsigned char channel_state) {
  switch(phase) {
    case ReceiverPhase::RECEIVE:
      return receive_phase(channel_state);
      break;
    case ReceiverPhase::AWAIT_BEGIN:
      return await_begin_phase(channel_state);
      break;
    case ReceiverPhase::CHECK:
      return check_phase(channel_state);
      break;
    case ReceiverPhase::FETCH_CHECKSUM:
      return fetch_checksum_phase(channel_state);
      break;
    default: // This should never happen
      return channel_state;
      break;
  }
}

bool Receiver::frame_available() {
  return frame_ready;
}

// Release the internal frame buffer if it contains a checked full frame.
bool Receiver::frame_pull(std::vector<unsigned char>& destination) {
  if(frame_ready) {
    // Copy in to destination byte by byte ......
    destination.clear();
    for(const auto& byte : byte_buffer) {
      destination.push_back(byte);
    }

    // Wipe the frame buffer... unset ready flag...
    frame_ready = !frame_ready;
    byte_buffer.clear();
    return true;
  } 
  else {
    return false;
  }
}
