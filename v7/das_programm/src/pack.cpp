// Functionalities to pack/unpack data to/from frames.
//
// Anatomy of a frame:
// BEGIN | CHECKSUM | DATA ... | END
//
// All data bytes representing control sequences
// must be prepended with ESCAPE control sequence.

#include <numeric>
#include <vector>
#include <iostream>

#include "common.hpp"
// #include "../extern/doctest.h"

// Create a frame from the data
void pack_frame(const std::vector<unsigned char>& data, std::vector<unsigned char>& frame) {
  
  frame.clear();
  // Push the BEGIN sequence.
  frame.push_back(static_cast<unsigned char>(ControlSeq::BEGIN));
  // Leave the CHECKSUM byte empty for now, we compute it later on.
  frame.push_back(0);

  // Push the data byte per byte. If a data byte is a control sequence,
  // prepend it with ESCAPE.
  for(const auto& byte : data) {
    if(is_control_sequence(byte)) {
      frame.push_back(static_cast<unsigned char>(ControlSeq::ESCAPE));
    }

    frame.push_back(byte);
  }

  // Push the END sequence.
  frame.push_back(static_cast<unsigned char>(ControlSeq::END));

  // Calculate checksum and set it.
  unsigned char checksum = std::accumulate(frame.begin() + 2, frame.end() - 1, 0);
  frame[1] = checksum;
}

void unpack_frame(const std::vector<unsigned char>& frame, std::vector<unsigned char>& data) {
  data.clear();

  // Get some iterators ignoring the BEGIN, CHECKSUM, and END sequences.
  auto it = frame.begin() + 2;
  auto end = frame.end() - 1;

  // "Ignore next control sequence" -> treat it as data!
  bool ignore_next_control_sequence = false;
  for(  ; it < end; it++ ) {

    // The byte we are looking at
    unsigned char byte = *it;

    // Handle control sequences
    if(is_control_sequence(byte)) {
      // Treat the control sequence as data if it should be ignored.
      if(ignore_next_control_sequence) {
        ignore_next_control_sequence = false;
      }
      else {
        if(byte == static_cast<unsigned char>(ControlSeq::ESCAPE)) {
          ignore_next_control_sequence = true;
          continue;
        }
        // Otherwise, do not skip this control sequence and skip to next byte and skip to next byte.
        ignore_next_control_sequence = false;
        continue;
      }
    }
    
    data.push_back(byte);
  }
}


