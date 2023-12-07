#include "receive.hpp"
#include "send.hpp"
#include "common.hpp"

#include <cstdio>
#include <vector>

bool operator==(const std::vector<unsigned char> a, const std::vector<unsigned char> b) {
  if(a.size() != b.size()) return false;

  for(unsigned i = 0; i < a.size(); i++) {
    if(a[i] != b[i]) return false;
  }

  return true;
}

void print_byte_vector(const std::vector<unsigned char>& vec) {
  for(const auto& byte : vec) {
    printf("0x%X, ", byte);
  }
  printf("\n");
}

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

  const std::vector<unsigned char> in_frame{ 0x2, 0xf, 0x7, 0x1, 0x1, 0x3 };
  std::vector<unsigned char> out_frame;

  unsigned char channel_state = 0x0;

  sender.read_frame(in_frame);

  unsigned i = 0;
  while(!receiver.frame_available()) {
    channel_state = sender.tick(channel_state);
    channel_state = receiver.tick(channel_state);

    fprintf(stderr, "%04d: ", i++);
    print_4_bits(channel_state);
  }

  receiver.frame_pull(out_frame);
  print_byte_vector(in_frame);
  print_byte_vector(out_frame);
}
