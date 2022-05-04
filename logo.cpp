#include "logo.hpp"

#include <iostream>
#include <iterator>
#include <limits>
#include <span>
#include <stdexcept>

#include <cstdint>
#include <cstdio>
#include <type_traits>

#include "png.hpp"
#include "readb.hpp"

using namespace std::string_literals;

constexpr auto magic_size = 9u;
constexpr auto dir_entry_size = 32u;
constexpr auto name_size = 24u;
constexpr auto image_magic_size = 8u;

void read_image_data(const std::span<std::byte> & data, const std::string & name, const std::string & input_filename)
{
    auto input = std::begin(data);

    if(auto magic = readstr(input, std::end(data), image_magic_size); magic != "MotoRun\0"s)
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

    if(auto magic = readstr(input, std::end(data), magic_size); magic != "MotoLogo\0"s)
        throw std::runtime_error{"Error reading " + input_filename + ": not a Moto logo.bin file"};

    auto directory_size = readb<std::uint32_t>(input, std::end(data), std::endian::little);

    const auto num_images = (directory_size - magic_size - sizeof(directory_size)) / dir_entry_size;

    std::vector<Image> images;

    for(auto i = 0u; i < num_images; ++i)
    {
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

std::vector<std::byte> write_image(const std::string & filename)
{
    std::vector<std::byte> data;
    auto output = std::back_inserter(data);

    auto im = read_png(filename);

    writestr("MotoRun\0"s, image_magic_size, output);
    writeb(static_cast<std::uint16_t>(im.width), output, std::endian::big);
    writeb(static_cast<std::uint16_t>(im.height), output, std::endian::big);

    for(auto row = 0u; row < im.height; ++row)
    {
        auto non_rle_buffer = std::vector<std::byte>{};
        auto write_non_rle = [&non_rle_buffer, &output]()
        {
            if(std::empty(non_rle_buffer))
                return;

            std::uint16_t count = std::size(non_rle_buffer) / 3u;
            if(count >= 0x1000)
                throw std::logic_error {"Too many non-RLE pixels"};
            writeb(count, output, std::endian::big);

            for(auto i = 0u; i < std::size(non_rle_buffer); i += 3)
            {
                writeb(non_rle_buffer[i + 2], output); // B
                writeb(non_rle_buffer[i + 1], output); // G
                writeb(non_rle_buffer[i + 0], output); // R
            }
            non_rle_buffer.clear();
        };
        auto write_rle =[&output](std::byte r, std::byte g, std::byte b, std::uint16_t count)
        {
            if(count >= 0x1000)
                throw std::logic_error {"Too many RLE pixels"};
            count |= 0x8000;
            writeb(count, output, std::endian::big);

            writeb(b, output);
            writeb(g, output);
            writeb(r, output);
        };

        for(auto col = 0u; col < im.width;)
        {
            auto current_r = static_cast<std::byte>(im.image_data[(row * im.width + col) * 3]);
            auto current_g = static_cast<std::byte>(im.image_data[(row * im.width + col) * 3 + 1]);
            auto current_b = static_cast<std::byte>(im.image_data[(row * im.width + col) * 3 + 2]);

            std::uint16_t count = 1u;

            for(auto x = col + 1; x < im.width && count < 0x1000; ++x, ++count)
            {
                auto r = static_cast<std::byte>(im.image_data[(row * im.width + x) * 3]);
                auto g = static_cast<std::byte>(im.image_data[(row * im.width + x) * 3 + 1]);
                auto b = static_cast<std::byte>(im.image_data[(row * im.width + x) * 3 + 2]);

                if(r != current_r || g != current_g || b != current_b)
                    break;
            }

            if(count > 2)
            {
                write_non_rle();
                write_rle(current_r, current_g, current_b, count);
                col += count;
            }
            else
            {
                non_rle_buffer.emplace_back(current_r);
                non_rle_buffer.emplace_back(current_g);
                non_rle_buffer.emplace_back(current_b);
                if(std::size(non_rle_buffer) / 3 == 0x1000)
                    write_non_rle();
                ++col;
            }
        }

        write_non_rle();
    }

    return data;
}

void write_logo(const std::vector<std::string> & filenames, const std::string & output_filename)
{
    std::uint32_t header_size = std::size(filenames) * dir_entry_size + magic_size + sizeof(std::uint32_t);

    std::vector<std::byte> data(header_size, std::byte{0xFF});
    auto output = std::begin(data);

    writestr("MotoLogo\0"s, magic_size, output);
    writeb(header_size, output, std::endian::little);

    auto round_to_mod512 = [](auto i) -> decltype(i)
    {
        auto diff = 512u - (i % 512u);
        return i + (diff == 512u ? 0u : diff);
    };

    for(auto && filename: filenames)
    {
        auto image_data = write_image(filename);

        if(std::size(image_data) > std::numeric_limits<std::uint32_t>::max())
            throw std::runtime_error{"Error writing " + filename + " compressed image size is too large"};

        if(std::size(data) + std::size(image_data) > std::numeric_limits<std::uint32_t>::max())
            throw std::runtime_error{"Error writing " + filename + " total image size is too large"};

        // save our output iterator in case the resize reallocates it
        auto output_offset = std::distance(std::begin(data), output);

        data.resize(round_to_mod512(std::size(data)), std::byte{0xFF});
        auto offset = static_cast<std::uint32_t>(std::size(data));
        data.insert(std::end(data), std::begin(image_data), std::end(image_data));

        output = std::begin(data) + output_offset;

        auto ext_start = filename.find_last_of('.');
        auto dir_end = filename.find_last_of("/\\");
        if(dir_end == std::string::npos)
            dir_end = 0;
        auto name = filename.substr(dir_end, ext_start - dir_end);

        if(std::size(name) > name_size - 1)
            throw std::runtime_error{"Error writing " + name + " filename exceeds maximum length(" + std::to_string(name_size - 1) + " characters)"};

        writestr(name, name_size, output);
        writeb(offset, output, std::endian::little);
        writeb(static_cast<uint32_t>(std::size(image_data)), output, std::endian::little);

        std::cout<<"Wrote "<<name<<'\n';
    }

    std::ofstream{output_filename, std::ios::binary}.write(reinterpret_cast<const char *>(std::data(data)), std::size(data));
}
