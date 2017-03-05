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
  // Needs to be a pointer, because it is otherwise copy-constructed and thus copied and
  // the iterators then point to the copy and not to the schedule itself.
  Flights *available_flights = &schedule->flights_from_on(frontier->descr.dest, current_day);
  DEBUG(
	if(available_flights->size() > 0)
	  for(Flights::iterator i = available_flights->begin(); i != available_flights->end(); ++i)
	    printf("Schedule(day=%d, dept=%d): %d -> %d $%d\n", current_day, frontier->descr.dest, i->dept, i->dest, i->cost);
  )
  if(available_flights->size() < 1){
    DEBUG(printf("no more flights from city=%d on day=%d\n", frontier->descr.dest, current_day));
    return false;
  }
  FlightsGenerator flights_iter(*available_flights);
  if(!grow_step_from_iter(flights_iter)){
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
    DEBUG(track_start->print());
    return grow_trek();
  }else if(!visited.visited_all()){
    DEBUG(printf("infeasuble route, does not go through all cities\n"));
    DEBUG(track_start->print());
    return grow_trek();
  }else{
    return track_start;
  }
}


void BacktrackingWayGenerator::rollback_days(int n)
{
  if(n > current_day)
    n = current_day;
  for(int i = 0; i < n; ++i){
    del_track_step();
    --current_day;
  }
}


// Steps back n days just to make a new proposal.  You can return to
// where you were by calling the regrow_pregrown method with the pointer
// to the start of the track you shrunk the path by.
Track *BacktrackingWayGenerator::stepback_days(int n)
{
  if(n < 1) return nullptr;
  if(n > current_day)
    n = current_day;
  for(int i = 0; i < n - 1; ++i){
    step_back_track_step();
    --current_day;
  }
  Track *stepped_back_track_start = step_back_track_step();
  --current_day;
  stepped_back_track_start->prev_element = nullptr;
  return stepped_back_track_start;
}


Track *BacktrackingWayGenerator::step_back_track_step()
{
  visited.unvisit(frontier->descr.dest);
  Track *old_frontier = frontier;
  frontier = frontier->prev_element;
  return old_frontier;
}


void BacktrackingWayGenerator::regrow_pregrown(Track *start, int days)
{
  if(days < 1) return;
  if(days > current_day)
    days = current_day;
  Track *old = stepback_days(days);
  current_day = current_day + days;
  old->forward_dispose();
  frontier->next_element = start;
  start->prev_element = frontier;
  Track *i;
  for(i = start; i; i = i->next_element)
    visited.visit(i->descr.dest);
  for(i = start; i->next_element; i = i->next_element);
  frontier = i;
}
