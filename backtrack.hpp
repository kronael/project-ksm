#ifndef BACKTRACK_H
#define BACKTRACK_H

#include "trackstep.hpp"
#include "schedule.hpp"
#include "destination_bitmap.hpp"

class BacktrackingWayGenerator
{
  Schedule *schedule;
  Track *track_start;
  Track *frontier;
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
  Track *grow_trek();
};

#endif
