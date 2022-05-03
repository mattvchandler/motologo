cmake_minimum_required (VERSION 3.14.2...3.18)
project(MotoLogo VERSION 1.0.0 LANGUAGES CXX)
set(PROJECT_DESCRIPTION "Moto log.bin unpacker / packer")
set(AUTHOR "Matthew Chandler <tardarsauce@gmail.com>")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-Wall)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

set(default_build_type "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release")
endif()

find_package(cxxopts REQUIRED)
find_package(PNG REQUIRED)

add_library(common OBJECT
    png.cpp
    logo.cpp
    )
add_executable(png2logo
    png2logo.cpp
    $<TARGET_OBJECTS:common>
    )

add_executable(logo2png
    logo2png.cpp
    $<TARGET_OBJECTS:common>
    )

target_include_directories(common PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(png2logo PNG::PNG)
target_link_libraries(logo2png PNG::PNG)
