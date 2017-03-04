#ifndef BACKTRACK_H
#define BACKTRACK_H

#include "trackstep.hpp"
#include "schedule.hpp"
#include "destination_bitmap.hpp"

struct BacktrackingWayGenerator
{
  Schedule *schedule;
  Track *track_start;
  Track *frontier;
  int min_cost_so_far;
  DestinationBitmap visited;
  // Current day points to the day of last departure in the track pointed to by frontier.
  int current_day;
  void add_track_step(int dest, int cost, FlightsGenerator flights);
  FlightsGenerator del_track_step();
  bool grow_step();
  bool grow_step_from_iter(FlightsGenerator flights);
  bool regrow_step();

public:
  BacktrackingWayGenerator(Schedule& new_schedule, Track& new_track_start, Track& new_frontier) :
    current_day(0), schedule(&new_schedule), track_start(&new_track_start), frontier(&new_frontier),
    visited(new_schedule.destination_count, new_track_start.descr.dest) {};
  BacktrackingWayGenerator(Schedule& new_schedule, int starting_city) :
    current_day(0), schedule(&new_schedule), visited(new_schedule.destination_count, 0){
    Flights *available_flights = &schedule->flights_from_on(starting_city, 0);
    DEBUG(
	  if(available_flights->size() > 0)
	    for(Flights::iterator i = available_flights->begin(); i != available_flights->end(); ++i)
	      printf("Schedule(day=%d, dept=%d): %d -> %d $%d\n", 0, starting_city, i->dept, i->dest, i->cost);
    )
    FlightsGenerator flights_iter(*available_flights);
    TrackStep initial_trackstep(starting_city, starting_city, 0, flights_iter);
    track_start = new Track(initial_trackstep);
    frontier = track_start;
  }
  Track *grow_trek();
  void rollback_days(int n);
  Track *get_frontier(){
    return frontier;
  }
};

#endif
