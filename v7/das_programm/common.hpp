#pragma once

enum class ControlSeq : unsigned char {
  ESCAPE = 0x1,
  BEGIN  = 0x2,
  END    = 0x3
};
