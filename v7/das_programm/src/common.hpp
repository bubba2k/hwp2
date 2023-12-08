#pragma once

#include <vector>

enum class ControlSeq : unsigned char {
  ESCAPE = 0x1,
  BEGIN  = 0x2,
  END    = 0x3
};

bool is_control_sequence(unsigned char byte);
