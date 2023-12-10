#include "receive.hpp"
#include "send.hpp"
#include "common.hpp"
#include "pack.hpp"

#include <string>
#include <cstdio>
#include <vector>

// Print the 4 LSBs
void print_4_bits(const unsigned char bits) {
  for(int i = 3; i >= 0; i--) {
    fprintf(stderr, "%d ", (bits >> i) & 0x1 );
  }
  fprintf(stderr, "\n");
}

int main() {
  Sender sender;
  Receiver receiver;

  // std::vector<unsigned char> in_frame{ 0x2, 0xf, 0x7, 0x1, 0x1, 0x3 };

  std::vector<unsigned char> out_frame_buffer, in_frame_buffer, out_data_buffer;
  std::vector<unsigned char> in_data_buffer{ 0x1, 0x2, 0x3, 0x4, 0x5 };

  unsigned char channel_state = 0x0;

  pack_frame(in_data_buffer, in_frame_buffer);;
  sender.read_frame(in_frame_buffer);

  unsigned i = 0;
  while(!receiver.frame_available()) {
    channel_state = receiver.tick(channel_state);
    channel_state = sender.tick(channel_state);

    fprintf(stderr, "%04d: ", i);
    print_4_bits(channel_state);

    i++;
  }

  receiver.frame_pull(out_frame_buffer);

  unpack_frame(out_frame_buffer, out_data_buffer);

  print_byte_vector(stdout, std::to_string(i) + "_data_in:\t", in_data_buffer);
  print_byte_vector(stdout, std::to_string(i) + "_frame_in:\t", in_frame_buffer);
  print_byte_vector(stdout, std::to_string(i) + "_frame_out:\t", out_frame_buffer);
  print_byte_vector(stdout, std::to_string(i) + "_data_out:\t", out_data_buffer);
}
