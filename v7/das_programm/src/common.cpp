#include "common.hpp"
#include <vector>

bool is_control_sequence(unsigned char byte) {
  return  (byte == static_cast<unsigned char>(ControlSeq::BEGIN)) ||
          (byte == static_cast<unsigned char>(ControlSeq::END))   ||
          (byte == static_cast<unsigned char>(ControlSeq::ESCAPE));
}

bool operator==(const std::vector<unsigned char> a, const std::vector<unsigned char> b) {
  if(a.size() != b.size()) return false;

  for(unsigned i = 0; i < a.size(); i++) {
    if(a[i] != b[i]) return false;
  }

  return true;
}

void print_byte_vector(FILE *stream, const std::string prefix, const std::vector<unsigned char>& vec) {
  fprintf(stream, "%s\n", prefix.c_str());
  for(const auto& byte : vec) {
    fprintf(stream, "0x%X, ", byte);
  }
  printf("\n");
}
