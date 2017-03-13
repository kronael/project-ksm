#include "flight.hpp"

// Serves to allow sorting of flights in TxtSchedule.
bool has_lower_cost(Flight x, Flight y){
  return x.cost < y.cost;
}
