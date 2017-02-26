#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>


class GrowthError : public std::exception
{
  const char* msg;
public:
  GrowthError(const char *message) : msg(message) {}
  const char* what(){
    return msg;
  }
};


class AllocError : public std::exception
{
  const char* msg;
public:
  AllocError(const char *message) : msg(message) {}
  const char* what(){
    return msg;
  }
};

#endif
