#include <cstdlib>
#include <stdio.h>

#include "backtrack.hpp"
#include "debug.hpp"
#include "schedule.hpp"
#include "trackstep.hpp"

int main()
{
  TxtSchedule ms;
  int starting_city = ms.load_flights_from_file("data_15.prep");

  Flights *available_flights_ptr = ms.flights_from_on(starting_city, 0);
  Flights available_flights = *available_flights_ptr;
  DEBUG(
	if(available_flights.size() > 0)
	  for(Flights::iterator i = available_flights.begin(); i != available_flights.end(); ++i)
	    printf("Schedule(day=%d, dept=%d): %d -> %d $%d\n", 0, starting_city, i->dept, i->dest, i->cost);
	)
  FlightsGenerator flights_iter(available_flights);
  TrackStep initial_trackstep(starting_city, starting_city, 0, flights_iter);
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
