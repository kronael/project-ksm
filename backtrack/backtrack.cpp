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
	DEBUG(printf("would close the cycle prematurely dest=%d\n", flight_dest));
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


// If cutoff is struck, reconnect the old track.
bool BacktrackingWayGenerator::regrow_step(Track *chopped)
{
  // If we're back at the start, it means there are no possible routes
  // from the first airport.
  DEBUG(printf("regrowing current_day=%d\n", current_day));
  if(current_day <= 0)
    return false;
  FlightsGenerator frontier_flights_iter = del_track_step();
  if(!grow_step_from_iter(frontier_flights_iter)){
    // There are no flights available from the current destination on this day.
    // We are not allowed to cross the cutoff.
    // if(current_day <= cutoff_day){
    //   printf("cutoff\n");
    //   // Better yet, should just reconnect one track step.
    //   // chopped->print();
    //   // if(chopped)
    //   // 	frontier = frontier->connect(chopped);
    //   //return true;
    // }else{
      --current_day;
      //}
    return regrow_step(chopped);
  }
  return true;
}


Track *BacktrackingWayGenerator::grow_trek(Track *chopped)
{
  while(1){
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
    if(!regrow_step(chopped)){
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
      continue;
    }else if(!visited.visited_all()){
      DEBUG(printf("infeasuble route, does not go through all cities\n"));
      DEBUG(track_start->print());
      continue;
    }else{
      return track_start;
    }
  }
  throw "should not get here";
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
// where you were by calling the regrow_pregrown method with the
// pointer to the start of the track you shrunk the path by.  Inserts
// a blocker to the frontier after stepping back to mark that the
// track before the stepped back day must remain unchanged.
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
  stepped_back_track_start->disconnect();
  cutoff_day = current_day;
  return stepped_back_track_start;
}


// Step back one day.  Returns the start of the tail of the path split
// from the track by one day.
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
  //printf("old\n");
  //old->print();
  current_day = current_day + days;
  old->forward_dispose();
  frontier = frontier->connect(start);
  Track *i;
  for(i = start; i; i = i->next_element)
    visited.visit(i->descr.dest);
  // Reset cutoff day.
  cutoff_day = 0;
}


// Updates a trackstep. If successful, returns cost of the original flight, otherwise, returns -1.
bool BacktrackingWayGenerator::exchange_trackstep(Track *track, unsigned int day, unsigned int new_dept, unsigned int new_dest)
{
  Flights *dept_flights = &schedule->flights_from_on(new_dept, day);
  for(Flights::iterator ia = dept_flights->begin(); ia != dept_flights->end(); ++ia)
    if(ia->dest == new_dest){
      FlightsGenerator flights_iter(*dept_flights);
      track->descr = TrackStep(new_dept, new_dest, ia->cost, flights_iter);
      return true;
    }

  return false;
}

/* Exchanges the flights at day dept_day departure and arrival
   airports.  At middle_dept_day, switch the link originating there.
   At the day before, change to have destination at the destination
   node of the first_dept_day original link.  The flight on day
   first_dept_day + 1 is exchanged for a link from former destination
   of a flight on first_dept_day to the destination of the flight on
   first_dept_day.
  
   Returns true if it succeeded (all the changes are doable).
   Returns false if it failed.
*/
bool BacktrackingWayGenerator::switch_flight(unsigned int all_days, unsigned int middle_switch_dept_day)
{
  unsigned int cur_dept_day = all_days;
  Track *i;
  for(i = frontier; i->prev_element; i = i->prev_element){
    if(cur_dept_day <= middle_switch_dept_day) break;
    --cur_dept_day;
  }
  // To switch, there needs to be at least one flight before middle_switch_dept_day flight.
  if(!i->prev_element) return false;
  // To switch, we need to exclude the first phony TrackStep (TODO fix).
  if(!i->prev_element->prev_element) return false;
  // To switch, there needs to be at least one flight after middle_switch_dept_day flight.
  if(!i->next_element) return false;
  DEBUG(printf("trying to switch middle=%d: %d <-> %d <-> %d <-> %d\n", middle_switch_dept_day, i->prev_element->descr.dept, i->descr.dept, i->descr.dest, i->next_element->descr.dest));

  TrackStep old_middle = i->descr;
  if(!exchange_trackstep(i, cur_dept_day, i->descr.dest, i->descr.dept)){
    return false;
  }else{
    // Reconnect before-middle flight day to a different destination: the new departure of middle flight day.
    TrackStep old_before = i->prev_element->descr;
    if(!exchange_trackstep(i->prev_element, cur_dept_day - 1, i->prev_element->descr.dept, i->descr.dept)){
      // Switch middle day back.
      i->descr = old_middle;
      return false;
    }else{
      if(!exchange_trackstep(i->next_element, cur_dept_day + 1, i->descr.dest, i->next_element->descr.dest)){
	// Switch middle and before back.
	i->descr = old_middle;
	i->prev_element->descr = old_before;
	return false;
      }else{
	return true;
      }
    }
  }
}
