#include "cityhash.hpp"

#include <cstdlib>
#include <string>

#include "debug.hpp"


int hashtag_in(char *str)
{
    return (str[0]-ALPHABET_START)*26*26+(str[1]-ALPHABET_START)*26+ (str[2]-ALPHABET_START);
}

std::string hashtag_out(int code)
{
    std::string str;
    str = "   ";
    str[0] = ALPHABET_START + (code / 676);
    str[1] = ALPHABET_START + ((code % 676) / 26);
    str[2] = ALPHABET_START + (code % 26);
    return str.c_str();
}
