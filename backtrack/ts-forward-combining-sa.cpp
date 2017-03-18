#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <random>

#include "backtrack.hpp"
#include "debug.hpp"
#include "schedule.hpp"
#include "trackstep.hpp"
#include "trekoptimum.hpp"
#include "types.hpp"

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
  std::cout << "took=" << timer.elapsed() / 1e6 << "s to load data" << std::endl;
  timer.reset();

  ms.sort_flights();
  std::cout << "took=" << timer.elapsed() / 1e6 << "s to sort data" << std::endl;

  #define EACH_N 100000
  Timer optimum_timer;
  Track *t = nullptr;
  Track *last = nullptr;
  TrekOptimum optimum;
  int i, j;
  int times[EACH_N];
  double temperature = 1000;
  int rollback_days;
  int last_state_cost = 100000000;
  #define TEMP_JUMP 1000000
  std::negative_binomial_distribution<int> days_distribution(1, 0.1);
  std::uniform_int_distribution<int> uniform_days(2, ms.schedule_days - 2);
  std::uniform_real_distribution<double> uniform_real(0, 1);

  bool regrown_flights;
  bool switched_flights;
  while(1){
    BacktrackingWayGenerator btg(ms, starting_city);
    t = btg.grow_trek();
    do{
      regrown_flights = false;
      switched_flights = false;

      if(t){
	if(uniform_real(GENERATOR) < 0.99){
	  rollback_days = days_distribution(GENERATOR);
	  DEBUG(printf("stepping back days=%d\n", rollback_days));
	  btg.stepback_days(rollback_days);
	  t = btg.grow_trek(nullptr, true);
	  regrown_flights = true;
	}else{
	  for(unsigned int i = 0; i < 10; ++i){
	    rollback_days = uniform_days(GENERATOR);
	    if(btg.switch_flight(rollback_days)){
	      DEBUG(printf("swapping flights days=%d back\n", rollback_days));
	      switched_flights = true;
	      break;
	    }
	  }
	  if(!switched_flights) continue;
	  btg.get_start()->next_element->fix_total_cost();
	  t = btg.get_start();
	}
      }
      timer.reset();
      // DEBUG(t->frontier()->validate(*btg.get_start()->next_element, argv[1]));
      // Completed a whole run, start over.
      if(!t){
	DEBUG(printf("all exhausted, running again\n"));
	continue;
      }

      if(optimum.update(*t, btg.get_frontier()->total_cost)){
	// Advance into the new optimum.
	std::cout << "took=" << optimum_timer.elapsed() / 1000 << "ms to find new optimum cost=" << btg.get_frontier()->total_cost << std::endl;
	optimum_timer.reset();
	DEBUG(t->frontier()->validate(*btg.get_start()->next_element, argv[1]));
      }else{
	// Explore non-optimal states.
	double transition_prob = exp(-(btg.frontier->total_cost - last_state_cost) / temperature);
	if(uniform_real(GENERATOR) < transition_prob){
	  DEBUG(printf("move from=%d to=%d\n", last_state_cost, btg.frontier->total_cost));
	  last_state_cost = btg.frontier->total_cost;
	}else{
	  DEBUG(printf("not stepping into from=%d to=%d\n", last_state_cost, btg.frontier->total_cost));
	  if(regrown_flights){
	    btg.stepback_days(rollback_days);
	    t = btg.grow_trek();
	  }
	  if(switched_flights){
	    if(!btg.switch_flight(rollback_days)){
	      throw new FlightExchangeError();
	      printf("failed to switch back\n");
	    }
	  }
	}
      }

      if(j >= TEMP_JUMP){
	temperature = temperature / float(2);
	btg.exchange_track(optimum.minimum->start()->duplicate());
	printf("new temp=%f\n", temperature);
	if(temperature < 10){
	  printf("zero temperature: we're done\n");
	  exit(EXIT_SUCCESS);
	}
	j = 0;
      }
      ++j;


      if(i == 0)
	std::cout << "took=" << timer.elapsed() << "us to generate the first track\n";
      if(i >= EACH_N){
	std::cout << "took=" << mean(times, EACH_N) << "us average to generate one track\n";
	i = 0;
      }
      times[i++] = timer.elapsed();
    } while(t);
  }
}
