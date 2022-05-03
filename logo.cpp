#include "logo.hpp"

#include <fstream>
#include <iostream>
#include <span>
#include <sstream>
#include <stdexcept>

#include <cmath>
#include <cstdint>
#include <type_traits>

#include "png.hpp"
#include "readb.hpp"

using namespace std::string_literals;

void read_image_data(const std::span<std::byte> & data, const std::string & name, const std::string & input_filename)
{
    auto input = std::begin(data);

    constexpr auto magic_size = 8u;
    if(auto magic = readstr(input, std::end(data), magic_size); magic != "MotoRun\0"s)
        throw std::runtime_error{"Error reading " + name + " from " + input_filename + ": bad identifier"};

    auto width = readb<std::uint16_t>(input, std::end(data), std::endian::big);
    auto height = readb<std::uint16_t>(input, std::end(data), std::endian::big);
    Image im{width, height};
    im.name = name + ".png";

    for(auto px = std::begin(im.image_data); px < std::end(im.image_data) && input < std::end(data);)
    {
        auto count = readb<std::uint16_t>(input, std::end(data), std::endian::big);
        if(count & 0x7000u)
            throw std::runtime_error{"Error reading " + name + " from " + input_filename + ": bad RLE count"};

        bool repeat = count & 0x8000u;
        count &= 0x0FFFu;

        auto write_pix = [&px, &im, &name, &input_filename](std::uint8_t c)
        {
            if(px == std::end(im.image_data))
                throw std::runtime_error{"Error reading " + name + " from " + input_filename + ": too many pixels"};
            *px++ = c;
        };

        if(repeat)
        {
            auto b = readb<std::uint8_t>(input, std::end(data));
            auto g = readb<std::uint8_t>(input, std::end(data));
            auto r = readb<std::uint8_t>(input, std::end(data));

            for(auto i = 0u; i < count; ++i)
            {
                write_pix(r);
                write_pix(g);
                write_pix(b);
            }
        }
        else
        {
            for(auto i = 0u; i < count; ++i)
            {
                auto b = readb<std::uint8_t>(input, std::end(data));
                auto g = readb<std::uint8_t>(input, std::end(data));
                auto r = readb<std::uint8_t>(input, std::end(data));

                write_pix(r);
                write_pix(g);
                write_pix(b);
            }
        }
    }

    std::cout<<"Extracted: "<<im.name<<" ("<<im.width<<"x"<<im.height<<")\n";
    write_png(im);
}

void read_logo(const std::string & input_filename)
{
    auto data = read_file(input_filename);
    auto input = std::begin(data);

    constexpr auto magic_size = 9u;
    if(auto magic = readstr(input, std::end(data), magic_size); magic != "MotoLogo\0"s)
        throw std::runtime_error{"Error reading " + input_filename + ": not a Moto logo.bin file"};

    auto directory_size = readb<std::uint32_t>(input, std::end(data), std::endian::little);

    constexpr auto dir_entry_size = 32u;
    const auto num_images = (directory_size - magic_size - sizeof(directory_size)) / dir_entry_size;

    std::vector<Image> images;

    for(auto i = 0u; i < num_images; ++i)
    {
        constexpr auto name_size = 24u;
        auto name = readstr(input, std::end(data), name_size);
        auto offset = readb<std::uint32_t>(input, std::end(data), std::endian::little);
        auto size = readb<std::uint32_t>(input, std::end(data), std::endian::little);

        name.resize(name.find_first_of('\0'));

        if(offset < directory_size || offset + size > std::size(data))
            throw std::runtime_error{"Error reading " + name + " from " + input_filename + ": bad offset and size"};

        // TODO: save the order in the name somehow?
        read_image_data(std::span{std::begin(data) + offset, std::begin(data) + offset + size}, name, input_filename);
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
