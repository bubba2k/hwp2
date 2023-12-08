// Functionalities to pack/unpack data to/from frames.
//
// Anatomy of a frame:
// BEGIN | CHECKSUM | DATA ... | END
//
// All data bytes representing control sequences
// must be prepended with ESCAPE control sequence.

#include <vector>
#include <algorithm>

#include "common.hpp"

// Create a frame from the data
void pack_frame(const std::vector<unsigned char>& data, std::vector<unsigned char> frame) {
  
}
