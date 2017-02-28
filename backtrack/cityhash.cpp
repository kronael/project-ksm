#include "cityhash.hpp"

#include <cstdlib>
#include <string>

#include "debug.hpp"


int hashtag_in(std::string str)
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
    return  str;
}

int stol(std::string str)
{
  char *end_pos;
  char **end_ptr = &end_pos;
  int ans = strtol(str.c_str(), end_ptr, 10);
  DEBUG(printf("read integer=%d\n", ans));
  if(*end_pos != '\0')
    throw NumberFormatError("number format error: invalid integer conversion");
  return ans;
}
