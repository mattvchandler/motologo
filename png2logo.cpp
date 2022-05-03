#include <iostream>
#include <stdexcept>
#include <string>

#include <cxxopts.hpp>

#include "png.hpp"
#include "logo.hpp"

struct Args
{
    std::vector<std::string> input_filenames;
    std::string output_filename;
};

std::optional<Args> get_args(int argc, char * argv[])
{
    cxxopts::Options options{argv[0], "Pack PNG files into a Moto logo.bin file"};

    // TODO: inputs may need to be ordered and have specific names. Some are probably required too
    try
    {
        options.add_options()
            ("h,help",   "Show this message and quit")
            ("o,output", "output filename. Default filename is logo.bin", cxxopts::value<std::string>()->default_value("logo.bin"), "OUTPUT")
            ("input",    "Input filenames", cxxopts::value<std::vector<std::string>>());

        options.parse_positional({"input"});
        options.positional_help("INPUT_PNG_FILES ...");

        auto args = options.parse(argc, argv);

        if(args.count("help"))
        {
            std::cerr<<options.help()<<'\n';
            return {};
        }

        Args output_args;
        output_args.input_filenames = args["input"].as<std::vector<std::string>>();
        output_args.output_filename = args["output"].as<std::string>();

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
        std::vector<Image> images;
        for(auto & i: args->input_filenames)
            images.emplace_back(read_png(i));

        write_logo(images, args->output_filename);
    }
    catch(const std::runtime_error & e)
    {
        std::cerr<<e.what()<<'\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
