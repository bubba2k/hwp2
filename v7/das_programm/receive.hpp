#pragma once

#include <vector>

enum class ReceiverPhase {
  AWAIT_BEGIN,
  FETCH_CHECKSUM,
  RECEIVE,
  CHECK,
  SEND_ACK,
};

class Receiver {
  std::vector<unsigned char> byte_buffer;
  unsigned char bit_buffer;

  ReceiverPhase phase;
  unsigned int n_ticks_ack_sent;
  unsigned int n_bits_received;
  bool last_clock;
  bool ignore_next_control_sequence;

  bool frame_ready;

  // Helpers
  bool read_clock(unsigned char channel_state);
  unsigned char get_data_bits(unsigned char channel_state);

  // State machine phases
  unsigned char await_begin_phase(unsigned char channel_state);
  unsigned char fetch_checksum_phase(unsigned char channel_state);
  unsigned char receive_phase(unsigned char channel_state);
  unsigned char check_phase(unsigned char channel_state);
  unsigned char send_ack_phase(unsigned char channel_state);

  public:
  bool frame_available();
  bool frame_pull(std::vector<unsigned char>& destination);
  unsigned char tick(unsigned char channel_state);
  Receiver();
};
