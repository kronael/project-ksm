#ifndef CITYHASH_H
#define CITYHASH_H

#define ALPHABET_START 'A'
#define HASH_SIZE 26 * 26 * 26

#include <exception>
#include <string>



class NumberFormatError : std::exception
{
  const char* msg;
public:
  NumberFormatError(const char *message) : msg(message) {}
  const char* what(){
    return msg;
  }
};


int hashtag_in(std::string str);
std::string hashtag_out(int code);
int stol(std::string str);

#endif
