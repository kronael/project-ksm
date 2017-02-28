#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <cstdlib>
#include <string.h>

#include "debug.hpp"
#include "destination_bitmap.hpp"
#include "spliceit.hpp"
#include "flight.hpp"
#include "cityhash.hpp"

class Schedule
{
public:
  unsigned int max_destination = HASH_SIZE;
  unsigned int destination_count;
  DestinationBitmap destination_counter;
  unsigned int schedule_days;
  Schedule() : destination_counter(HASH_SIZE, 0) {}
  virtual Flights *flights_from_on(int dept, int day) {};
};


class TxtSchedule : public Schedule
{
  std::vector< std::vector<Flights> > schedule;
  void add_flight(int dept, int dest, int day, int cost){
    DEBUG(printf("adding flight dept=%d dest=%d day=%d cost=%d\n", dept, dest, day, cost));
    if(schedule_days < day){
      std::vector<Flights> new_day;
      Flights empty_flights;
      new_day.assign(max_destination, empty_flights);
      schedule.push_back(new_day);
      ++schedule_days;
    }
    // Count destinations.
    if(!destination_counter.visited(dest))
      destination_counter.visit(dest);
    Flight new_flight(dept, dest, cost);
    schedule[day][dept].push_back(new_flight);
  }
public:
  TxtSchedule(){
    // Init for day 0.
    schedule_days = 0;
    std::vector<Flights> new_day;
    Flights empty_flights;
    new_day.assign(max_destination, empty_flights);
    schedule.push_back(new_day);
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

    return starting_city;
  }
  virtual Flights *flights_from_on(int dept, int day){
    if(day > schedule_days || dept > max_destination){
      Flights *empty = new Flights();
      return empty;
    }
    return &schedule[day][dept];
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
  virtual Flights *flights_from_on(int dept, int day){
    if(day >= schedule_days || dept == 6){
      Flights *empty = new Flights();
      return empty;
    }
    return &schedule[day][dept];
  }
};

#endif
