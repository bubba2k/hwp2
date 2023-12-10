// Define the doctest entry point
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../extern/doctest.h"

#include <array>
#include <vector>
#include <cstdio>
#include <iostream>

#include "pack.hpp"
#include "common.hpp"

bool operator==(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b) {
  if(a.size() != b.size()) return false;

  for(unsigned i = 0; i < a.size(); i++) {
    if(a[i] != b[i]) return false;
  }
  
  return true;
}

TEST_CASE("Pack/Unpack check") 
{

  static const std::vector<std::vector<unsigned char>> data_vec_array{
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6 },
    { 0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6 },
    { 21, 213, 21, 45, 252, 111, 204, 21},
    { 94, 123, 24, 29, 95, 100, 183, 233 }
  };

  SUBCASE("Pack/Unpack check") {
    std::vector<unsigned char> frame_buffer, out_buffer;

    for(unsigned i = 0; i < data_vec_array.size(); i++) {

      // print_byte_vector(stderr, "Expected: ", data_vec_array[i]);
      pack_frame(data_vec_array[i], frame_buffer);
      // print_byte_vector(stderr, "Frame buffer: ", frame_buffer);
      unpack_frame(frame_buffer, out_buffer);
      // print_byte_vector(stderr, "Data buffer: ", out_buffer);

      CHECK(data_vec_array[i] == out_buffer);
    }
  }

  SUBCASE("Pack check") {
    std::vector<unsigned char> frame_buffer;
    std::vector<unsigned char> data{ 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
    std::vector<unsigned char> expected{  static_cast<unsigned char>(ControlSeq::BEGIN), 
                                          0xa + 0xb + 0xc + 0xd + 0xe + 0xf,
                                          0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
                                          static_cast<unsigned char>(ControlSeq::END) };

    pack_frame(data, frame_buffer);

    std::cerr << "Frame buffer size: " << frame_buffer.size() << std::endl;
    print_byte_vector(stderr, "Expected: ", expected);
    print_byte_vector(stderr, "Data buffer: ", data);
    print_byte_vector(stderr, "Frame buffer: ", frame_buffer);

    CHECK(frame_buffer == expected);
  }
}

