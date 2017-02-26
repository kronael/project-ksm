#include <cstdlib>
#include <stdio.h>

#include "schedule.hpp"
#include "backtrack.hpp"
#include "trackstep.hpp"

int main()
{
  MockSchedule ms;
  Flights *fs_ptr = ms.flights_from_on(0, 0);
  Flights fs = *fs_ptr;
  for(Flights::iterator i = fs.begin(); i != fs.end(); ++i)
    printf("Day0 (0): %d -> %d $%d\n", i->dept, i->dest, i->cost);
  Flights *fs1_ptr = ms.flights_from_on(1, 1);
  Flights fs1 = *fs1_ptr;
  for(Flights::iterator i = fs1.begin(); i != fs1.end(); ++i)
    printf("Day1 (1): %d -> %d $%d\n", i->dept, i->dest, i->cost);
  Flights *fs2_ptr = ms.flights_from_on(4, 2);
  Flights fs2 = *fs2_ptr;
  for(Flights::iterator i = fs2.begin(); i != fs2.end(); ++i)
    printf("Day2 (4): %d -> %d $%d\n", i->dept, i->dest, i->cost);

  Flights *available_flights_ptr = ms.flights_from_on(0, 0);
  Flights available_flights = *available_flights_ptr;
  FlightsGenerator flights_iter(available_flights);
  TrackStep initial_trackstep(0, 0, 0, flights_iter);
  Track trek(initial_trackstep, nullptr);
  BacktrackingWayGenerator btg(ms, trek, trek);

  Track *t;
  do{
    t = btg.grow_trek();
    if(!t){
      printf("all done\n");
      exit(EXIT_SUCCESS);
    }
    printf("OUT BEGIN\n");
    t->next_element->print();
    printf("OUT END\n");
   
  }while(t);
}
