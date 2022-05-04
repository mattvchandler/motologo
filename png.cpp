#include "png.hpp"

#include <stdexcept>

#include <cstring>

class Png
{
public:
    Png()
    {
        std::memset(&png_, 0, sizeof(png_));
        png_.version = PNG_IMAGE_VERSION;
        png_.format = PNG_FORMAT_RGB;
    }
    ~Png()
    {
        png_image_free(&png_);
    }

    operator png_image * () { return &png_; }
    operator const png_image * () const { return &png_; }

    png_image * operator->() { return &png_; }
    const png_image * operator->() const { return &png_; }

    png_image & get() { return png_; }
    const png_image & get() const { return png_; }

private:
    png_image png_;
};

Image read_png(const std::string & input_filename)
{
    Png png_img;

    if(!png_image_begin_read_from_file(png_img, input_filename.c_str()))
        throw std::runtime_error {"Error reading PNG: " + std::string{png_img->message}};

    png_img->format = PNG_FORMAT_RGB;

    Image img{png_img->width, png_img->height};
    if(std::size(img.image_data) != PNG_IMAGE_SIZE(png_img.get()))
        throw std::runtime_error {"PNG size mismatched"};

    if(!png_image_finish_read(png_img, nullptr, std::data(img.image_data), PNG_IMAGE_ROW_STRIDE(png_img.get()), nullptr))
        throw std::runtime_error {"Could not finish reading PNG: " + std::string{png_img->message}};

    return img;
}

void write_png(const Image & img)
{
    Png png_img;
    png_img->width = img.width;
    png_img->height = img.height;

    if(!png_image_write_to_file(png_img, img.name.c_str(), false, std::data(img.image_data), PNG_IMAGE_ROW_STRIDE(png_img.get()), nullptr))
        throw std::runtime_error {"Error writing PNG: " + std::string{png_img->message}};
}
