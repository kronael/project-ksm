#include "debug.hpp"
#include "types.hpp"
#include "trackstep.hpp"

struct TrekOptimum
{
  Track *minimum;
  int minimal_cost;
  TrekOptimum() : minimum(nullptr), minimal_cost(MAX_INT) {}
  bool update(Track& track_start, unsigned int total_cost){
    DEBUG(printf("comparing\n"));
    if(minimal_cost > total_cost){
      DEBUG(printf("disposing\n"));
      if(minimum)
	minimum->dispose();
      DEBUG(printf("duplicating\n"));
      minimum = track_start.duplicate();
      minimal_cost = total_cost;
      return true;
    }
    return false;
  }
};
