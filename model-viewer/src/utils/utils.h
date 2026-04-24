#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <numbers>
#include <algorithm>
#include <vector>

bool starts_with(const std::string& str, const std::string& start);
bool ends_with(const std::string& str, const std::string& end);
std::string read_file(const std::string& filename);
std::vector<unsigned char> read_png_file(const char* filename, int& width, int& height, int& bit_depth);

#endif
