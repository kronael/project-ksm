#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

#include "backtrack.hpp"
#include "debug.hpp"
#include "schedule.hpp"
#include "trackstep.hpp"
#include "trekoptimum.hpp"

bool DEBUGGING_ENABLED = false;

#include <iostream>
#include <chrono>

class Timer
{
  typedef std::chrono::high_resolution_clock clock_;
  std::chrono::time_point<clock_> beg_;

public:
  Timer() : beg_(clock_::now()) {}
  void reset(){
    beg_ = clock_::now();
  }
  long elapsed() const{ 
    return std::chrono::duration_cast<std::chrono::microseconds>(clock_::now() - beg_).count();
  }
};


double mean(const int* x, int size)
{
  int sum = 0;
  for(int i = 0; i < size; ++i)
    sum += x[i];
  return sum / size;
}


int main(int argc, char **argv)
{
  if(argc < 1)
    exit(1);
  
  char *debugging = getenv("DEBUG");
  if(debugging && (strcmp(debugging, "1") == 0))
    DEBUGGING_ENABLED = true;

  Timer timer;
  TxtSchedule ms;
  int starting_city = ms.load_flights_from_file(argv[1]);

  BacktrackingWayGenerator btg(ms, starting_city);
  btg.frontier->print();
  
  std::cout << "took=" << timer.elapsed() / 1e6 << "s to load data" << std::endl;

  #define EACH_N 100000
  Timer optimum_timer;
  Track *t;
  TrekOptimum optimum;
  int i;
  int times[EACH_N];
  do{
    timer.reset();
    t = btg.grow_trek();
    if(!t){
      printf("all done\n");
      exit(EXIT_SUCCESS);
    }
    if(optimum.update(*t->next_element, *btg.get_frontier())){
      std::cout << "took=" << optimum_timer.elapsed() / 1000 << "ms to find new optimum\n";
      optimum_timer.reset();
      printf("OUT BEGIN\n");
      t->next_element->print();
      printf("OUT END\n");
    }
    if(i == 0)
      std::cout << "took=" << timer.elapsed() << "us to generate the first track\n";
    if(i >= EACH_N){
      std::cout << "took=" << mean(times, EACH_N) << "us average to generate one track\n";
      i = 0;
    }
    times[i++] = timer.elapsed();
  } while(t);
}
