#include <iostream>
#include <vector>
#include <cstdio>
#include <numeric>

#include "receive.hpp"
#include "common.hpp"

// TODO: Implement fetch_checksum_phase, check_phase
//
// Masks for channel state    76543210
#define DATA_BIT_MASK  0b00000011
#define ACK_BIT_MASK   0b00000100
#define CLK_BIT_MASK   0b00001000
// BEGIN | CHECKSUM | DATA... | END

Receiver::Receiver() {
  phase = ReceiverPhase::AWAIT_BEGIN;
  n_bits_received = 0;
  bit_buffer = 0;
  last_clock = false;
  ignore_next_control_sequence = false;
  frame_ready = false;
  n_ticks_ack_sent = 0;
  send_ack = false;
}

// Read current state of the clock.
bool Receiver::read_clock(unsigned char channel_state) {
  return channel_state & CLK_BIT_MASK;
}

// Read the state of the data bits. (The two least significant bits)
unsigned char Receiver::get_data_bits(unsigned char channel_state) {
  return channel_state & DATA_BIT_MASK;
}

unsigned char Receiver::await_begin_phase(unsigned char channel_state) {
  unsigned char new_channel_state = channel_state;
  // Send ACK here (or not)
  if(send_ack) {
    new_channel_state |= ACK_BIT_MASK;
  }
  else {
    new_channel_state &= ~ACK_BIT_MASK;
  }

  // Only do something if the clock has changed.
  // If the clock hath not changed, we shall abort this tick.
  bool new_clock = read_clock(channel_state);
  if(new_clock == last_clock) {
    return new_channel_state;
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
      byte_buffer.push_back(bit_buffer);    // Store the byte
      phase = ReceiverPhase::RECEIVE;
    }

    fprintf(stderr, "RECV_AWAIT_BEGIN: Received 0x%x\n", bit_buffer);

    n_bits_received = 0;
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

    fprintf(stderr, "RECV_RECEIVE: Received 0x%x\n", bit_buffer);
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
            fprintf(stderr, "RECV_RECEIVE: Going to CHECK_PHASE\n");

            // Check if the checksum works out here...
            // Checksums exclude the first BEGIN byte, the checksum byte itself,
            // and the END byte. Note that the END byte is not in the buffer yet
            // so we accumulate all the way to the end.
            unsigned char calculated_checksum = 
              std::accumulate(byte_buffer.begin() + 2, byte_buffer.end(), 0);
            unsigned char received_checksum = byte_buffer[1];

            if(received_checksum == calculated_checksum) {     
              fprintf(stderr, "RECV_RECEIVE: Check success, sending ACK\n");
              phase = ReceiverPhase::AWAIT_BEGIN;
              send_ack = true;
              frame_ready = true;
            }
            else {
              fprintf(stderr, "RECV_RECEIVE: Check failed!\n");
              phase = ReceiverPhase::AWAIT_BEGIN;
              send_ack = false;
              frame_ready = false;
            }
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
    default: // This should never happen
      fprintf(stderr, "RECV_TICK: ERROR: Default case!!!\n");
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
