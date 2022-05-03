#ifndef IMAGE_HPP
#define IMAGE_HPP

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

    class Color_view
    {
    public:
        Color_view(std::uint8_t * ptr):
            r{ptr[0]},
            g{ptr[1]},
            b{ptr[2]},
            ptr_{ptr}
        {}

        Color_view() = delete;
        constexpr std::uint8_t & operator[](std::size_t i)
        {
            return ptr_[i];
        }

        std::uint8_t & r, & g, & b;

    private:
        std::uint8_t * ptr_;
    };

    class Const_color_view
    {
    public:
        Const_color_view(const std::uint8_t * ptr):
            r{ptr[0]},
            g{ptr[1]},
            b{ptr[2]},
            ptr_{ptr}
        {}

        Const_color_view() = delete;
        constexpr const std::uint8_t & operator[](std::size_t i) const
        {
            return ptr_[i];
        }

        const std::uint8_t & r, & g, & b;

    private:
        const std::uint8_t * ptr_;
    };

    Color_view get_pixel(std::size_t row, std::size_t col)
    {
        return Color_view{&image_data[(row * width + col) * 3]};
    }

    const Const_color_view get_pixel(std::size_t row, std::size_t col) const
    {
        return Const_color_view{&image_data[(row * width + col) * 3]};
    }

    std::size_t width{0};
    std::size_t height{0};
    std::vector<std::uint8_t> image_data;
};

#endif // IMAGE_HPP
