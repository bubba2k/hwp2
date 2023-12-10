#pragma once

#include <vector>

void pack_frame(const std::vector<unsigned char>& data, std::vector<unsigned char>& frame);
void unpack_frame(const std::vector<unsigned char>& frame, std::vector<unsigned char>& data);
