#ifndef FLIGHT_H
#define FLIGHT_H

#include <stdio.h>
#include <vector>

struct Flight
{
  unsigned int dept, dest, cost, day;
  Flight() :
    dept(0), dest(0), cost(0), day(0) {}
  Flight(unsigned int new_dept, unsigned int new_dest, unsigned int new_cost, unsigned int new_day) :
    dept(new_dept), dest(new_dest), cost(new_cost), day(new_day) {}
};


bool has_lower_cost(Flight x, Flight y);

typedef std::vector<Flight> Flights;
typedef Flights::iterator FlightsIterator;


class FlightsGenerator
{
  FlightsIterator end;
public:
  FlightsIterator current;
  FlightsGenerator() :
    end(nullptr), current(nullptr) {}
  FlightsGenerator(Flights& flights) :
    current(std::begin(flights)), end(std::end(flights)) {}
  virtual bool has_next(){
    current != end;
  }
  virtual bool is_valid(){
    current < end;
  }
  virtual FlightsIterator next(){
    return ++current;
  }
  void print(){
    printf("Iterator(%x, %x)\n", current, end);
    if(!is_valid())
      printf("Iterator(%x, %x): exhausted\n", current, end);
    for(Flights::iterator i = current; i < end; ++i)
      printf("Iterator(%x, %x): %d -> %d $%d\n", current, end, i->dept, i->dest, i->cost);
  }
};


class FlightsIteratorGenerator : public FlightsGenerator
{
  std::vector<Flights::iterator>::iterator end_it;
  std::vector<Flights::iterator>::iterator current_it;
public:
  FlightsIterator current;
  FlightsIteratorGenerator() :
    end_it(nullptr), current_it(nullptr), current(nullptr) {}
  FlightsIteratorGenerator(std::vector<Flights::iterator>& flights) :
    current_it(std::begin(flights)), current(*current_it), end_it(std::end(flights)) {}
  virtual bool has_next(){
    current_it != end_it;
  }
  virtual bool is_valid(){
    current_it < end_it;
  }
  virtual FlightsIterator next(){
    return *(++current_it);
  }
};

#endif
