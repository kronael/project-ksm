#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <algorithm>
#include <cstdlib>
#include <string.h>
#include <random>
#include <utility>

#include "csv.h"
#include "debug.hpp"
#include "destination_bitmap.hpp"
#include "spliceit.hpp"
#include "flight.hpp"
#include "cityhash.hpp"

extern std::default_random_engine GENERATOR;


struct Schedule
{
  unsigned int max_destination = HASH_SIZE;
  unsigned int schedule_days;
  virtual Flights& flights_from_on(int dept, int day) = 0;
  Schedule() {}
  Schedule(unsigned int new_max_destination, unsigned int new_schedule_days) :
    max_destination(new_max_destination), schedule_days(new_schedule_days) {} 
};


class TxtSchedule : public Schedule
{
  std::vector< std::vector<Flights> > schedule;
  std::vector< std::vector<Flights> > backwards_schedule;
  void add_flight(int dept, int dest, int day, int cost){
    //DEBUG(printf("adding flight dept=%d dest=%d day=%d cost=%d\n", dept, dest, day, cost));
    if(schedule_days < day){
      std::vector<Flights> new_day;
      new_day.resize(max_destination);
      for(; schedule_days < day; ++schedule_days){
	schedule.emplace_back(new_day);
	backwards_schedule.emplace_back(new_day);
      }
    }
    schedule[day][dept].emplace_back(Flight(dept, dest, cost, day));
    //linear_schedule.push_back(new_flight);
    backwards_schedule[day][dest].emplace_back(Flight(dest, dept, cost, day));
  }
public:
  Flights linear_schedule;
  TxtSchedule(){
    // Init for day 0.
    schedule_days = 0;
    std::vector<Flights> new_day;
    new_day.resize(max_destination);
    schedule.emplace_back(new_day);
    backwards_schedule.emplace_back(new_day);
  }

  // Sorts the flights by cost for each day and departure airport.
  void sort_flights(std::vector< std::vector<Flights> > *x = nullptr){
    if(!x) x = &schedule;
    for(auto day = x->begin(); day != x->end(); ++day)
      for(auto dept = day->begin(); dept != day->end(); ++dept)
	std::sort(dept->begin(), dept->end(), has_lower_cost);
  }

  // Sorts the backwards schedule by cost for each day and arrival airport.
  void sort_backwards_flights(){
    sort_flights(&backwards_schedule);
  }

  void sort_linear_flights(){
    std::sort(std::begin(linear_schedule), std::end(linear_schedule), has_lower_cost);
  }

  // Randomly choose 'number_of_schuffles' flights from every
  // available city on each day and place them to the start of the
  // list exchanging them for those which were at their original
  // positions.
  void _shuffle_single_flight(Flights& single_dept_schedule){
    // std::vector<double> d{0, (double)single_dept_schedule.size() / 10, (double)single_dept_schedule.size()};
    // std::vector<double> w{1, 0.1, 0};
    // std::piecewise_linear_distribution<double> distribution(d.begin(), d.end(), w.begin());

    if(single_dept_schedule.size() == 0){
      return;
    }else{
      std::bernoulli_distribution toss_coin(0.05);
      int toss = toss_coin(GENERATOR);
      if(toss) return;
      std::vector<double> cuts;
      std::vector<double> costs;
      costs.resize(single_dept_schedule.size());
      cuts.resize(single_dept_schedule.size());
      for(auto i = 0; i < single_dept_schedule.size(); ++i){
	costs[i] = exp(- (single_dept_schedule[i].cost / 10)^2);
	cuts[i] = i;
      }
      std::piecewise_linear_distribution<double> distribution(cuts.begin(), cuts.end(), costs.begin());
      //std::exponential_distribution<double> distribution(10);
      int random_index = std::lround(distribution(GENERATOR));
      if(random_index == 0){
	return;
      }else{
	if(random_index > single_dept_schedule.size())
	  random_index = single_dept_schedule.size();
	printf("picking i=%d\n", random_index);
	std::swap(single_dept_schedule[random_index], single_dept_schedule[0]);
      }
    }
  }

  void shuffle_flights(int number_of_schuffles, int from_day){
    std::bernoulli_distribution toss_coin(0.5);
    for(auto day = from_day; day < schedule.size(); ++day){
      int toss = toss_coin(GENERATOR);
      if(toss) continue;
      printf("shuffling at day=%d\n", day);
      for(auto dept = schedule[day].begin(); dept != schedule[day].end(); ++dept)
	_shuffle_single_flight(*dept);
    }
  }
  int load_flights_from_file(const char *input_filename);
  virtual Flights& flights_from_on(int dept, int day){
    if(day > schedule_days || dept > max_destination){
      Flights *empty = new Flights();
      return *empty;
    }
    return schedule[day][dept];
  }
  Flights& flights_to_on(int dest, int day){
    if(day > schedule_days || dest > max_destination){
      Flights *empty = new Flights();
      return *empty;
    }
    return backwards_schedule[day][dest];
  }
};


// Emulate a forward shedule, while being a backwards schedule.
class BackwardsSchedule : public Schedule
{
  TxtSchedule backing_schedule;
public:
  BackwardsSchedule() : Schedule() {}
  BackwardsSchedule(TxtSchedule& new_backing_schedule) :
    backing_schedule(new_backing_schedule),
    Schedule(new_backing_schedule.max_destination, new_backing_schedule.schedule_days) {}
  // Ivert the schedule.
  virtual Flights& flights_from_on(int dept, int day){
    return backing_schedule.flights_to_on(dept, day);
  }
};

#endif
