#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "flight.hpp"

class Schedule
{
public:
  unsigned int destination_count;
  unsigned int schedule_days;
  virtual Flights *flights_from_on(int dept, int day) {};
};

// class TxtSchedule : public Schedule
// {
  
// public:
//   TxtSchedule(const char* file);
  
// }


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
