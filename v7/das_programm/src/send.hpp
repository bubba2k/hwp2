#pragma once

#include <vector>

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

  bool _need_new_frame;
  bool _have_frame;

  unsigned char send_phase(unsigned char channel_state);
  unsigned char await_ack_phase(unsigned char channel_state);

  public:
  unsigned char tick(unsigned char last_read);
  bool need_frame();
  bool read_frame(const std::vector<unsigned char>& frame);
  Sender();
};

