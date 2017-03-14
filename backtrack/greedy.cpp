#include <stdio.h>

#include "greedy.hpp"
#include "flight.hpp"



void GreedyWayGenerator::flush_flights()
{
  visited = DestinationBitmap(schedule->schedule_days + 1);
  traveling_day.assign(schedule->schedule_days + 2, false);
  traveling_day[0] = true;
}


void GreedyWayGenerator::add_flight(const Flights::iterator& flight)
{
  visited.visit(flight->dest);
  traveling_day[flight->day + 1] = true;
  trek[flight->day + 1] = *flight;
  stack.push(flight);
  total_cost += flight->cost;
  DEBUG(printf("adding (%d) dept=%d, dest=%d, day=%d, cost=%d\n", visited.visited_count, flight->dept, flight->dest, flight->day, flight->cost));
}


// Returns false if there is no "current flight" anymore.
bool GreedyWayGenerator::del_last_flight()
{
  stack.pop();
  if(stack.empty()) return false;
  Flights::iterator last_flight(current_flight());
  DEBUG(printf("deleting dept=%d, dest=%d, day=%d, cost=%d\n", last_flight->dept, last_flight->dest, last_flight->day, last_flight->cost));
  visited.unvisit(last_flight->dest);
  traveling_day[last_flight->day + 1] = false;
  total_cost -= last_flight->cost;
  return true;
}


bool GreedyWayGenerator::can_flight_be_added(const Flight& flight)
{
  static int n;
  if(visited.visited(flight.dest)) return false;
  if(traveling_day[flight.day + 1]) return false;
  if(traveling_day[flight.day] && trek[flight.day].dest != flight.dept) return false;
  // If it is not the last day, etc.
  if(flight.day < schedule->schedule_days && traveling_day[flight.day + 2] && trek[flight.day + 2].dept != flight.dest) return false;
  return true;
}



FlightsGenerator GreedyWayGenerator::greedy_track()
{
  bool deleted_stack = stack.size() > 1;
  while(stack.size() > 1)
    del_last_flight();
  if(deleted_stack)
    ++current_flight();
  while(true){
    Flights::iterator *flight = &current_flight();
    if(*flight == schedule->linear_schedule.end()){
      if(!del_last_flight())
	return FlightsGenerator();
    }else if(can_flight_be_added(**flight)){
      add_flight(current_flight());
    }
    ++current_flight();

    if(visited.visited_all() && trek.front().dept == trek.back().dest){
      DEBUG(printf("BEGIN generated trek\n"));
      for(auto i = trek.begin(); i != trek.end(); ++i)
	DEBUG(printf("dept=%d, dest=%d, day=%d, cost=%d\n", i->dept, i->dest, i->day, i->cost));
      DEBUG(printf("END generated trek\n"));
      return FlightsGenerator(trek);
    }
  }
}
