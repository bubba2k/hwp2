// Define the doctest entry point
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../extern/doctest.h"

#include <array>
#include <vector>
#include <cstdio>
#include <iostream>

#include "pack.hpp"
#include "common.hpp"
#include "send.hpp"
#include "receive.hpp"

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
      pack_frame(data_vec_array[i], false, frame_buffer);
      // print_byte_vector(stderr, "Frame buffer: ", frame_buffer);
      bool eof = false;
      unpack_frame(frame_buffer, eof, out_buffer);
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

    pack_frame(data, false, frame_buffer);

    std::cerr << "Frame buffer size: " << frame_buffer.size() << std::endl;
    print_byte_vector(stderr, "Expected: ", expected);
    print_byte_vector(stderr, "Data buffer: ", data);
    print_byte_vector(stderr, "Frame buffer: ", frame_buffer);

    CHECK(frame_buffer == expected);
  }
}

TEST_CASE("Receive/Send test") {
  std::vector<unsigned char> frame_buffer, out_buffer;
  std::vector<unsigned char> data{ 0xa, 0xb, 0xc, 0xd, 0xe, 0xf };
  std::vector<unsigned char> expected_frame{  static_cast<unsigned char>(ControlSeq::BEGIN), 
                                        0xa + 0xb + 0xc + 0xd + 0xe + 0xf,
                                        0xa, 0xb, 0xc, 0xd, 0xe, 0xf,
                                        static_cast<unsigned char>(ControlSeq::END) };

  SUBCASE("One correct frame") {
    Receiver receiver;
    Sender sender;

    pack_frame(data, false, frame_buffer);
    sender.read_frame(frame_buffer, false);

    CHECK(frame_buffer == expected_frame);

    unsigned char channel_state = 0x0;
    unsigned i = 0;
    while(i++ < 100) {
      channel_state = receiver.tick(channel_state);
      channel_state = sender.tick(channel_state);
    }

    receiver.frame_pull(frame_buffer);
    CHECK(frame_buffer == expected_frame);

    bool eof;
    unpack_frame(frame_buffer, eof, out_buffer);
    CHECK(out_buffer == data);
  }

  SUBCASE("Multiple correct frames") {
    std::vector<std::vector<unsigned char>> data_blocks{
      { 123, 20, 43, 24, 56, 78, 79, 80, 81, 100 },
      { 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 123, 124, 100, 42, 32, 54, 100, 103, 1, 2, 3, 4, 88 },
      { 123, 20, 43, 24, 56, 78, 79, 80, 81, 100 },
    };

    Receiver receiver;
    Sender sender;
    unsigned data_block_index = 0;

    unsigned char channel_state = 0x0;
    while(data_block_index < data_blocks.size()) {
      if(sender.need_frame()) {
        pack_frame(data_blocks[data_block_index], false, frame_buffer);
        sender.read_frame(frame_buffer, false);
      }
      if(receiver.frame_available()) {
        receiver.frame_pull(frame_buffer);
        bool eof;
        unpack_frame(frame_buffer, eof, out_buffer);
        CHECK(out_buffer == data_blocks[data_block_index]);
        data_block_index += 1;
      }

      channel_state = receiver.tick(channel_state);
      channel_state = sender.tick(channel_state);
    }

  }
}

