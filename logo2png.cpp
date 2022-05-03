#include <iostream>
#include <stdexcept>
#include <string>

#include <cxxopts.hpp>

#include "png.hpp"
#include "logo.hpp"

struct Args
{
    std::string input_filename;
};

std::optional<Args> get_args(int argc, char * argv[])
{
    cxxopts::Options options{argv[0], "Unpack a Moto logo.bin file into PNGs"};

    try
    {
        options.add_options()
            ("h,help",   "Show this message and quit")
            ("input",    "Input filename", cxxopts::value<std::string>());

        options.parse_positional({"input"});
        options.positional_help("LOGO.BIN");

        auto args = options.parse(argc, argv);

        if(args.count("help"))
        {
            std::cerr<<options.help()<<'\n';
            return {};
        }

        Args output_args;
        output_args.input_filename = args["input"].as<std::string>();

        return output_args;
    }
    catch(const cxxopts::OptionException & e)
    {
        std::cerr<<options.help()<<'\n'<<e.what()<<'\n';
        return {};
    }
}

int main(int argc, char * argv[])
{
    auto args = get_args(argc, argv);
    if(!args)
        return EXIT_FAILURE;

    try
    {
        auto images = read_logo(args->input_filename);
        for(std::size_t i = 0; i < std::size(images); ++i)
            write_png(images[i], std::to_string(i) + ".png");
    }
    catch(const std::runtime_error & e)
    {
        std::cerr<<e.what()<<'\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
