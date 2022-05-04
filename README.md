# MotoLogo

Simple, no-frills command line tool for unpacking and repacking Moto android
logo.bin files.

This is primarily useful for changing the screen shown on boot on an unlocked
device (Your device has been unlocked and can't be trusted)

## Building

#### Dependencies

* [cxxopts](https://github.com/jarro2783/cxxopts)
* libpng
* CMake

On linux/unix, you could build with the following steps:

    git clone https://github.com/mattvchandler/motologo
    cd motologo
    mkdir build
    cd build
    cmake ..
    make
    cd ..

## Usage

#### logo2png

`logo2png path_to_logo.bin`

This will dump a set of .png images into the current directory

#### png2logo

`png2logo -o path_to_logo.bin image1.png image2.png ...`

Image filenames should be the same as dumped by logo2png for your device's
logo.bin. You should also probably keep the same image dimensions as the
original files, although I have had some success resizing some of the UI
elements.

Once satisfied with your new logo file, you can flash it to your device with

`fastboot flash logo logo.bin`

## Warning

If you choose to use this tool, you are modifying files used by your device's
bootloader. I can't guarantee that your phone will still boot, or even if it
won't be bricked. Good luck, though!

