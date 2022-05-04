#ifndef READB_HPP
#define READB_HPP

#include <array>
#include <bit>
#include <fstream>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include <cerrno>
#include <cstddef>
#include <cstring>

template<typename T> concept Byte_input_iter =
#ifdef _cpp_lib_concepts
std::input_iterator<T> &&
#else
// poor man's input_iterator
requires(T begin, T end)
{
    begin == end;
    ++begin;
    *begin = std::declval<decltype(*begin)>();
    *begin++ = std::declval<decltype(*begin)>();
    requires std::is_same_v<decltype(begin == end), bool>;
    requires std::is_same_v<decltype(++begin), T&>;
} &&
#endif
requires { requires sizeof(*std::declval<T>()) == 1; };

template<typename T> concept Byte_output_iter =
#ifdef _cpp_lib_concepts
std::output_iterator<T>;
#else
// poor man's output_iterator
requires(T begin)
{
    *begin = std::declval<decltype(*begin)>();
    ++begin;
    begin++;
    *begin++ = std::declval<decltype(*begin)>();
    requires std::is_same_v<decltype(++begin), T&>;
};
#endif

inline std::vector<std::byte> read_file(const std::string & filename)
{
    std::ifstream file{filename, std::ios::binary};
    if(!file)
        throw std::runtime_error{"Could not open input region file: " + filename + ". " + std::strerror(errno)};

    std::vector<std::byte> data;
    std::array<std::byte, 4096> buffer;

    do
    {
        file.read(reinterpret_cast<char *>(std::data(buffer)), std::size(buffer));
        data.insert(std::end(data), std::begin(buffer), std::begin(buffer) + file.gcount());
    } while(file);

    if(file.bad())
        throw std::runtime_error{"Could not open read region file: " + filename + ". " + std::strerror(errno)};

    return data;
}

template <typename T>
T bswap(T a)
{
    auto & buf = reinterpret_cast<std::byte(&)[sizeof(T)]>(a);

    if constexpr(sizeof(T) == 1)
        return a;
    else if constexpr(sizeof(T) == 2)
    {
        #if defined(__GNUC__) // also clang
        *reinterpret_cast<std::uint16_t *>(buf) = __builtin_bswap16(*reinterpret_cast<std::uint16_t *>(buf));
        #elif defined(_MSC_VER)
        *reinterpret_cast<std::uint16_t *>(buf) = _byteswap_ushort(*reinterpret_cast<std::uint16_t *>(&buf));
        #else
        std::swap(buf[0], buf[1]);
        #endif
    }
    else if constexpr(sizeof(T) == 4)
    {
        #if defined(__GNUC__) // also clang
        *reinterpret_cast<std::uint32_t *>(buf) = __builtin_bswap32(*reinterpret_cast<std::uint32_t *>(&buf));
        #elif defined(_MSC_VER)
        *reinterpret_cast<std::uint32_t *>(buf) = _byteswap_ulong(*reinterpret_cast<std::uint32_t *>(&buf));
        #else
        std::swap(buf[0], buf[3]);
        std::swap(buf[1], buf[2]);
        #endif
    }
    else if constexpr(sizeof(T) == 8)
    {
        #if defined(__GNUC__) // also clang
        *reinterpret_cast<std::uint64_t *>(buf) = __builtin_bswap64(*reinterpret_cast<std::uint64_t *>(&buf));
        #elif defined(_MSC_VER)
        *reinterpret_cast<std::uint64_t *>(buf) = _byteswap_uint64(*reinterpret_cast<std::uint64_t *>(&buf));
        #else
        std::swap(buf[0], buf[7]);
        std::swap(buf[1], buf[6]);
        std::swap(buf[2], buf[5]);
        std::swap(buf[3], buf[4]);
        #endif
    }
    else
        static_assert(sizeof(T) != 1 && sizeof(T) != 2 && sizeof(T) != 4 && sizeof(T) != 8, "unsupported type for bswap");

    return a;
}

template <typename T, Byte_input_iter InputIter> requires (!std::is_enum_v<T>)
T readb(InputIter & begin, InputIter end, std::endian endian = std::endian::little)
{
    T t;
    auto & buf = reinterpret_cast<std::byte(&)[sizeof(T)]>(t);
    for(auto && i: buf)
    {
        if(begin == end)
            throw std::runtime_error{"Unexpected end of input"};
        i = static_cast<std::byte>(*begin++);
    }
    if(std::endian::native != endian)
        t = bswap(t);
    return t;
}

template<typename E, Byte_input_iter InputIter> requires std::is_enum_v<E>
E readb(InputIter & begin, InputIter end, std::endian endian = std::endian::little)
{
    return static_cast<E>(readb_i<std::underlying_type_t<E>>(begin, end, endian));
}

template <Byte_input_iter InputIter>
std::string readstr(InputIter & begin, InputIter end, std::size_t len)
{
    std::string s(len, '\0');
    for(std::size_t i = 0; i < len; ++i)
    {
        if(begin == end)
            throw std::runtime_error{"Unexpected end of input"};
        s[i] = static_cast<char>(*begin++);
    }
    return s;
}

template <typename T, Byte_output_iter OutputIter> requires(!std::is_enum_v<T>)
void writeb(T t, OutputIter & out, std::endian endian = std::endian::little)
{
    if(std::endian::native != endian)
        t = bswap(t);
    auto & buf = reinterpret_cast<std::byte(&)[sizeof(T)]>(t);
    for(auto && i: buf)
        *out++ = i;
}

template <typename E, Byte_output_iter OutputIter> requires(std::is_enum_v<E>)
void writeb(E e, OutputIter & out, std::endian endian = std::endian::little)
{
    writeb(static_cast<std::underlying_type_t<E>>(e), out, endian);
}

template <Byte_output_iter OutputIter>
void writestr(const std::string_view & str, std::size_t len, OutputIter & out)
{
    for(auto i = 0u; i < len && i < std::size(str); ++i)
        *out++ = static_cast<std::byte>(str[i]);
    for(auto i = std::size(str); i < len; ++i)
        *out++ = static_cast<std::byte>('\0');
}

#endif // READB_HPP
