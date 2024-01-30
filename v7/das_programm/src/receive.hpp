#pragma once

#include <vector>

enum class ReceiverPhase {
  AWAIT_BEGIN,
  RECEIVE,
  SEND_ACK_AND_END
};

class Receiver {
  std::vector<unsigned char> byte_buffer;
  unsigned char bit_buffer;

  ReceiverPhase phase;
  unsigned int n_ticks_ack_sent;
  unsigned int n_bits_received;
  bool last_clock;
  bool ignore_next_control_sequence;
  bool send_ack;
  bool _done;
  bool _done_after_this_frame;

  bool frame_ready;

  // Helpers
  bool read_clock(unsigned char channel_state);
  unsigned char get_data_bits(unsigned char channel_state);

  // State machine phases
  unsigned char await_begin_phase(unsigned char channel_state);
  unsigned char receive_phase(unsigned char channel_state);
  unsigned char send_ack_and_end(unsigned char channel_state);

  public:
  inline bool is_done() const { return _done; }
  bool frame_available();
  bool frame_pull(std::vector<unsigned char>& destination);
  unsigned char tick(unsigned char channel_state);
  Receiver();
};
