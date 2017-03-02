#include "debug.hpp"
#include "trackstep.hpp"

#define MAX_INT 100000000

struct TrekOptimum
{
  Track *minimum;
  int minimal_cost;
  TrekOptimum() : minimum(nullptr), minimal_cost(MAX_INT) {}
  bool update(Track& track_start, Track& frontier){
    DEBUG(printf("comparing\n"));
    if(minimal_cost > frontier.total_cost){
      DEBUG(printf("disposing\n"));
      if(minimum)
	minimum->dispose();
      DEBUG(printf("duplicating\n"));
      minimum = track_start.duplicate();
      minimal_cost = frontier.total_cost;
      return true;
    }
    return false;
  }
};
