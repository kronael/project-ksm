#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <algorithm>
#include <cstdlib>
#include <string.h>
#include <random>
#include <utility>

#include "debug.hpp"
#include "destination_bitmap.hpp"
#include "spliceit.hpp"
#include "flight.hpp"
#include "cityhash.hpp"

extern std::default_random_engine GENERATOR;


struct Schedule
{
  unsigned int max_destination = HASH_SIZE;
  unsigned int destination_count;
  unsigned int schedule_days;
  virtual Flights& flights_from_on(int dept, int day) = 0;
};


class TxtSchedule : public Schedule
{
  DestinationBitmap destination_counter;
  std::vector< std::vector<Flights> > schedule;
  std::vector< std::vector<Flights> > backwards_schedule;
  void add_flight(int dept, int dest, int day, int cost){
    DEBUG(printf("adding flight dept=%d dest=%d day=%d cost=%d\n", dept, dest, day, cost));
    if(schedule_days < day){
      std::vector<Flights> new_day;
      Flights empty_flights;
      new_day.assign(max_destination, empty_flights);
      schedule.push_back(new_day);
      backwards_schedule.push_back(new_day);
      ++schedule_days;
    }
    // Count destinations.
    if(!destination_counter.visited(dest))
      destination_counter.visit(dest);
    Flight new_flight(dept, dest, cost);
    schedule[day][dept].push_back(new_flight);
    Flight new_reversed_flight(dest, dept, cost);
    backwards_schedule[day][dest].push_back(new_reversed_flight);
  }
public:
  TxtSchedule() : destination_counter(HASH_SIZE, 0){
    // Init for day 0.
    schedule_days = 0;
    std::vector<Flights> new_day;
    Flights empty_flights;
    new_day.assign(max_destination, empty_flights);
    schedule.push_back(new_day);
    backwards_schedule.push_back(new_day);
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

  // Returns starting city.
  int load_flights_from_file(const char *input_filename){
    char *line = nullptr;
    size_t len = 0;
    ssize_t read;
    int nfields;
    char **fields;

    // Open files.
    FILE* infile = fopen(input_filename, "r");
    if(infile == nullptr){
      printf("failed to open input file\n");
      exit(EXIT_FAILURE);
    }

    // First line is special (starting city, discard here).
    if((read = getline(&line, &len, infile)) == -1){
      printf("failed to read first line containing starting city\n");
      exit(EXIT_FAILURE);
    }
    char *start_chopped;
    DEBUG(printf("starting city=%s\n", line));
    chop(line, &start_chopped);
    DEBUG(printf("starting city=%s\n", start_chopped));
    int starting_city = hashtag_in(start_chopped);
    DEBUG(printf("starting city=%d\n", starting_city));
    free(start_chopped);

    while((read = getline(&line, &len, infile)) != -1){
      nfields = str_split(line, INPUT_SEPARATOR, &fields);
      // Include selected fields.
      DEBUG(printf("read fields %s, %s, %s, %s\n", fields[0], fields[1], fields[2], fields[3]));
      int dept = hashtag_in(fields[0]);
      int dest = hashtag_in(fields[1]);
      int day = stol(fields[2]);
      char *chopped;
      chop(fields[3], &chopped);
      int cost = stol(chopped);
      free(chopped);
      add_flight(dept, dest, day, cost);

      int i;
      for(i = 0; i < nfields; i++)
	free(fields[i]);
      free(fields);
    }

    fclose(infile);
    free(line);

    // Record destination count.
    destination_count = destination_counter.visited_count;
    DEBUG(printf("loaded data with destination_count=%d\n", destination_count));

    // Reverse the backwards schedule for the days to match.
    std::reverse(backwards_schedule.begin(), backwards_schedule.end());

    return starting_city;
  }
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
class BackwardsSchedule : public TxtSchedule
{
  // BackwardsSchedule(TxtSchedule& other_schedule) :
  //   destination_counter(other_schedule.destination_counter), schedule(other_schedule.schedule),
  //   backwards_schedule(other_schedule.schedule) {}

  virtual Flights& flights_from_on(int dept, int day){
    return flights_to_on(dept, day);
  }
};


class MockSchedule : public Schedule
{
  std::vector< std::vector<Flights> > schedule;
public:
  MockSchedule(){
    // Day0
    std::vector<Flights> day0;

    Flights zero_flights;

    Flights flights0;
    Flight f00(0, 1, 1);
    Flight f01(0, 2, 19);
    Flight f02(0, 3, 9);
    Flight f03(0, 6, 2);
    flights0.push_back(f00);
    flights0.push_back(f01);
    flights0.push_back(f02);
    flights0.push_back(f03);

    day0.assign(6, zero_flights);
    day0[0] = flights0;
    schedule.push_back(day0);

    // Day1
    std::vector<Flights> day1;
    day1.assign(6, zero_flights);

    Flights flights11;
    Flight f10(1, 2, 20);
    Flight f11(1, 4, 9);
    flights11.push_back(f10);
    flights11.push_back(f11);

    day1[1] = flights11;

    Flights flights12;
    Flight f12(2, 1, 4);
    Flight f13(2, 0, 8);
    flights12.push_back(f12);
    flights12.push_back(f13);
    day1[2] = flights12;
    schedule.push_back(day1);

    // Day2
    std::vector<Flights> day2;
    day2.assign(6, zero_flights);
    
    Flights flights20;
    Flight f22(0, 3, 4);
    flights20.push_back(f22);
    day2[0] = flights20;

    Flights flights22;
    Flight f21(2, 5, 20);
    flights22.push_back(f21);
    day2[2] = flights22;

    Flights flights24;
    Flight f20(4, 0, 20);
    Flight f23(4, 2, 2);
    flights24.push_back(f20);
    flights24.push_back(f23);
    day2[4] = flights24;

    schedule.push_back(day2);

    destination_count = 6;
    schedule_days = 3;
  }
  virtual Flights& flights_from_on(int dept, int day){
    if(day >= schedule_days || dept == 6){
      Flights *empty = new Flights();
      return *empty;
    }
    return schedule[day][dept];
  }
};

#endif
