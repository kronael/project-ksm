#include <stdio.h>

#include "backtrack.hpp"

#include "trackstep.hpp"
#include "flight.hpp"
#include "exceptions.hpp"
#include "schedule.hpp"
#include "debug.hpp"

/*! Adds the next available route from the flights iterator.
*/ 
void BacktrackingWayGenerator::add_track_step(int dest, int cost, FlightsGenerator flights)
{
  TrackStep grown_track_step(frontier->descr.dest, dest, cost, flights);
  frontier = frontier->enlarge(grown_track_step);
  visited.visit(dest);
}


/*! Select one flight from the flights iterator, inserts it into
    current track (advancing frontier) and advances the iterator, which is
    saved along with the track.  If the new city is already visited, skip it and isert the next one.
    \param flights the flights interator from which to take the next flight.
    \return true if track extended, false if no further flights available in the flights iterator.
*/
bool BacktrackingWayGenerator::grow_step_from_iter(FlightsGenerator flights)
{
  DEBUG(printf("growing from iter: current frontier: "));
  DEBUG(frontier->print());
  DEBUG(flights.print());
  if(!flights.is_valid()){
    return false;
  }else{
    int flight_dest = flights.current->dest;
    int flight_cost = flights.current->cost;
    flights.next();
    if(!visited.visited(flight_dest)){
      if(flight_dest == track_start->descr.dest && !visited.almost_all_visited()){
	DEBUG(printf("would close the loop prematurely dest=%d\n", flight_dest));
	return grow_step_from_iter(flights);
      }else{
	add_track_step(flight_dest, flight_cost, flights);
	return true;
      }
    }else{
      DEBUG(printf("already visited dest=%d\n", flight_dest));
      return grow_step_from_iter(flights);
    }
  }
}


/*! Grows the track by a single flight.  The flight is taken from a freshly
    constructed iterator which is saved along with the new step.
*/
bool BacktrackingWayGenerator::grow_step()
{
  DEBUG(printf("growing day=%d\n", current_day));
  Flights *available_flights = schedule->flights_from_on(frontier->descr.dest, current_day);
  DEBUG(
	if(available_flights->size() > 0)
	  for(Flights::iterator i = available_flights->begin(); i != available_flights->end(); ++i)
	    printf("Schedule(day=%d, dept=%d): %d -> %d $%d\n", current_day, frontier->descr.dest, i->dept, i->dest, i->cost);
	)
  if(available_flights->size() < 1){
    // GrowthError error("can't grow any further");
    // throw error;
    DEBUG(printf("no more flights from city=%d on day=%d\n", frontier->descr.dest, current_day));
    return false;
  }
  FlightsGenerator flights_iter(*available_flights);
  if(!grow_step_from_iter(flights_iter)){
    // GrowthError error("can't grow any further");
    // throw error;
    return false;
  }
  ++current_day;
  return true;
}


FlightsGenerator BacktrackingWayGenerator::del_track_step()
{
  FlightsGenerator frontier_flights_iter(frontier->descr.flights_iter);
  visited.unvisit(frontier->descr.dest);
  frontier = frontier->shrink();
  return frontier_flights_iter;
}


bool BacktrackingWayGenerator::regrow_step()
{
  // If we're back at the start, it means there are no possible routes
  // from the first airport.
  DEBUG(printf("regrowing current_day=%d\n", current_day));
  if(current_day <= 0)
    return false;
  FlightsGenerator frontier_flights_iter = del_track_step();
  if(!grow_step_from_iter(frontier_flights_iter)){
    // There are no flights available from the current destination on this day.
    --current_day;
    return regrow_step();
  }
  return true;
}


Track *BacktrackingWayGenerator::grow_trek()
{
  // On the start of processing, first day, move forward.
  if(current_day == 0){
    grow_step();
    // Grow the route until we can.
    while(frontier->descr.dest != track_start->descr.dest && grow_step()) {}
    DEBUG(printf("grown until we can: whole track so far\n"));
    DEBUG(track_start->print());
    if(frontier->descr.dest == track_start->descr.dest && visited.visited_all()){
      DEBUG(printf("we have a possible route\n"));
      return track_start;
    }
    DEBUG(printf("infeasible route obtained\n"));
  }

  // On further processing runs, we start with a complete route, or
  // the route is not complete, we must regrow either way.
  // If we can't, we're done.
  DEBUG(printf("regrowing step\n"));
  if(!regrow_step()){
    DEBUG(printf("we're done\n"));
    return nullptr;
  }
  // Regrowing might have shrunk the track, or if the track is not complete.
  DEBUG(printf("growing until we can after regrowth\n"));
  while(frontier->descr.dest != track_start->descr.dest && grow_step()) {}
  // Once the track is complete, return it if it is a valid track, otherwise
  // Do the iteration again.
  if(frontier->descr.dest != track_start->descr.dest){
    DEBUG(printf("infeasible route, does not form a cycle with dept=%d dest=%d\n", track_start->descr.dest, frontier->descr.dest));
    return grow_trek();
  }else if(visited.visited_all()){
    DEBUG(printf("infeasuble route, does not go through all cities\n"));
    return grow_trek();
  }else{
    return track_start;
  }
}
