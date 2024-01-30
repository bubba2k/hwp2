#pragma once

#include <vector>

void pack_frame(const std::vector<unsigned char>& data, bool signal_eof, std::vector<unsigned char>& frame);
void unpack_frame(const std::vector<unsigned char>& frame, bool& signal_eof, std::vector<unsigned char>& data);
