#ifndef TYPES_H
#define TYPES_H

#include <random>

#define MAX_INT 1000000000

struct FlightExchangeError : std::exception {
  const char* what() const noexcept{
    return "failed to switch flights\n";
  }
};

std::default_random_engine GENERATOR(1987);

#endif
