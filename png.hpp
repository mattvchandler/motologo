#ifndef PNG_HPP
#define PNG_HPP

#include <string>

#include <png.h>

#include "image.hpp"

Image read_png(const std::string & input_filename);
void write_png(const Image & img, const std::string & output_filename);

#endif // PNG_HPP
