#ifndef BACKTRACK_H
#define BACKTRACK_H

#include "trackstep.hpp"
#include "schedule.hpp"
#include "destination_bitmap.hpp"

#define SUCCESS 0
#define NO_MORE_FLIGHTS 1
#define CUTOFF_STRUCK 2

struct BacktrackingWayGenerator
{
  Schedule *schedule;
  Track *track_start;
  Track *frontier;
  int cutoff_day;
  DestinationBitmap visited;
  // Current day points to the day of last departure in the track pointed to by frontier.
  int current_day;
  void add_track_step(int dest, int cost, FlightsGenerator flights);
  FlightsGenerator del_track_step();
  bool grow_step();
  bool grow_step_from_iter(FlightsGenerator flights);
  bool regrow_step(Track *chopped);
  bool exchange_trackstep(Track *track, unsigned int day, unsigned int new_dept, unsigned int new_dest);

public:
  BacktrackingWayGenerator(Schedule& new_schedule, Track& new_track_start, Track& new_frontier) :
    cutoff_day(0), current_day(0), schedule(&new_schedule), track_start(&new_track_start), frontier(&new_frontier),
    visited(new_schedule.destination_count, new_track_start.descr.dest) {};
  BacktrackingWayGenerator(Schedule& new_schedule, int starting_city) :
    cutoff_day(0), current_day(0), schedule(&new_schedule), visited(new_schedule.destination_count, 0){
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
  BacktrackingWayGenerator(Schedule& new_schedule, unsigned int starting_city, FlightsGenerator& trek_flights) :
    cutoff_day(0), current_day(new_schedule.schedule_days), schedule(&new_schedule), visited(new_schedule.destination_count, 0){
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

    if(!trek_flights.has_next())
      return;
    trek_flights.next();

    Flights::iterator i;
    unsigned int day = 0;
    for( ; trek_flights.has_next(); trek_flights.next()){
      i = trek_flights.current;
      printf("dept=%d, dest=%d, day=%d, cost=%d\n", i->dept, i->dest, i->day, i->cost);
      FlightsGenerator flights(schedule->flights_from_on(i->dept, day++));
      TrackStep step(i->dept, i->dest, i->cost, flights);
      frontier = frontier->enlarge(step);
    }
  }
  Track *grow_trek(Track *chopped = nullptr);
  void rollback_days(int n);
  Track *get_frontier(){
    return frontier;
  }
  Track *get_start(){
    return track_start;
  }
  Track *stepback_days(int n);
  Track *step_back_track_step();
  void regrow_pregrown(Track *trek_start, int days);
  bool switch_flight(unsigned int all_days, unsigned int middle_switch_dept_day);
};

#endif
