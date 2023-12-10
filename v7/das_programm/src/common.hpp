#pragma once

#include <vector>
#include <string>

enum class ControlSeq : unsigned char {
  ESCAPE = 0x1,
  BEGIN  = 0x2,
  END    = 0x3
};

bool is_control_sequence(unsigned char byte);
bool operator==(const std::vector<unsigned char> a, const std::vector<unsigned char> b);
void print_byte_vector(FILE *stream, const std::string prefix, const std::vector<unsigned char>& vec);
