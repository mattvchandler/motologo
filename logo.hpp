#ifndef LOGO_HPP
#define LOGO_HPP

#include <string>
#include <vector>

#include "image.hpp"

void read_logo(const std::string & input_filename);
void write_logo(const std::vector<Image> & images, const std::string & output_filename);
#endif // LOGO_HPP
