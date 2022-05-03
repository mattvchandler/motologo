#ifndef IMAGE_HPP
#define IMAGE_HPP

#include <string>
#include <vector>

#include <cstdint>

struct Image
{
public:
    Image() = default;
    Image(std::size_t w, std::size_t h)
    {
        set_size(w, h);
    }

    void set_size(std::size_t w, std::size_t h)
    {
        width = w;
        height = h;

        image_data.resize(w * h * 3);
    }

    std::size_t width{0};
    std::size_t height{0};
    std::vector<std::uint8_t> image_data;

    std::string name;
};

#endif // IMAGE_HPP
