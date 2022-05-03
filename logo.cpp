#include "logo.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <cmath>
#include <cstdint>
#include <cstring>

#include "readb.hpp"

Image read_image_data(std::istream & input)
{
    unsigned short width = 0, height = 0;
    Image im{width, height};

    for(std::size_t row = 0; row < height; ++row)
    {
        for(std::size_t col = 0; col < width; ++col)
        {
            // auto color_view = im.get_pixel(row, col);
        }
    }

    return im;
}

std::vector<Image> read_logo(const std::string & input_filename)
{
    std::ifstream input;
    input.exceptions(std::ios_base::failbit);
    try
    {
        input.open(input_filename, std::ios_base::binary);
    }
    catch(std::ios_base::failure &)
    {
        throw std::runtime_error {"Could not open: " + input_filename + " for reading: " + std::strerror(errno) };
    }
    input.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    try
    {
        constexpr auto num_images = 0u;

        std::vector<Image> images;

        for(std::uint32_t i = 0; i < num_images; ++i)
            images.emplace_back(read_image_data(input));

        return images;
    }
    catch(std::ios_base::failure & e)
    {
        if(input.bad())
            throw std::runtime_error{"Error reading SRF: could not read file: " + std::string{std::strerror(errno)}};
        else
            throw std::runtime_error{"Error reading SRF: unexpected end of file"};
    }
}

void write_image(std::ostream & out, const Image & img)
{
}

void write_logo(const std::vector<Image> & images, const std::string & output_filename)
{
    std::ofstream out(output_filename, std::ios_base::binary);
    if(!out)
        throw std::runtime_error {"Could not open: " + output_filename + " for writing: " + std::strerror(errno) };

    out.exceptions(std::ios_base::badbit | std::ios_base::failbit);

    try
    {
    }
    catch(std::ios_base::failure & e)
    {
        throw std::runtime_error{"Error writing SRF: " + std::string{std::strerror(errno)}};
    }
}
