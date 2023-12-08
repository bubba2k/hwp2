#include "common.hpp"
#include <vector>

bool is_control_sequence(unsigned char byte) {
  return  (byte == static_cast<unsigned char>(ControlSeq::BEGIN)) ||
          (byte == static_cast<unsigned char>(ControlSeq::END))   ||
          (byte == static_cast<unsigned char>(ControlSeq::ESCAPE));
}
