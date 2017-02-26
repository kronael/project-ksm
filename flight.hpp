#ifndef FLIGHT_H
#define FLIGHT_H

#include <stdio.h>
#include <vector>

struct Flight
{
  int dept, dest, cost;
  Flight() :
    dept(0), dest(0), cost(0) {}
  Flight(int new_dept, int new_dest, int new_cost) :
    dept(new_dept), dest(new_dest), cost(new_cost) {}
};


#define Flights std::vector<Flight>
#define FlightsIterator Flights::iterator


class FlightsGenerator
{
  FlightsIterator end;
public:
  FlightsIterator current;
  FlightsGenerator(Flights& flights) :
    current(std::begin(flights)), end(std::end(flights)) {}
  bool has_next(){
    current != end;
  }
  bool is_valid(){
    current < end;
  }
  FlightsIterator next(){
    return ++current;
  }
  void print(){
    printf("Iterator(%x, %x)\n", current, end);
    if(!is_valid()) printf("Iterator(%x, %x): exhausted\n", current, end);
    for(Flights::iterator i = current; i < end; ++i)
      printf("Iterator(%x, %x): %d -> %d $%d\n", current, end, i->dept, i->dest, i->cost);
  }
};

#endif
