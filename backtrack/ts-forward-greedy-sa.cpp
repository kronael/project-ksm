#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>

#include "backtrack.hpp"
#include "debug.hpp"
#include "schedule.hpp"
#include "trackstep.hpp"
#include "trekoptimum.hpp"
#include "greedy.hpp"

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
  ms.sort_flights();
  ms.sort_linear_flights();

  std::cout << "took=" << timer.elapsed() / 1e6 << "s to load data" << std::endl;

  #define EACH_N 100000
  Timer optimum_timer;
  Track *t = nullptr;
  Track *last = nullptr;
  TrekOptimum optimum;
  int i, j;
  int times[EACH_N];
  double temperature = 1000000;
  int rollback_days;
  int last_state_cost = 100000000;
  #define TEMP_JUMP 100000
  std::negative_binomial_distribution<int> days_distribution(2, 0.2);
  std::uniform_real_distribution<double> uniform_real(0, 1);

  while(1){
    //t = btg.grow_trek();
    //t->frontier()->validate(*t->start()->next_element, argv[1]);
    GreedyWayGenerator gtg(ms, starting_city);

    do{
      FlightsGenerator g = gtg.greedy_track();
      if(!g.has_next()){
	printf("problem\n");
	break;
	exit(1);
      }

      BacktrackingWayGenerator btg(ms, starting_city, g);
      t = btg.get_start();
      t->frontier()->validate(*t->next_element, argv[1]);
      printf("total_cost=%d\n", t->frontier()->total_cost);

      // if(t){
      // 	//t->print();
      // 	//rollback_days = days_distribution(GENERATOR);
      // 	//last = btg.stepback_days(rollback_days);
      // }
      timer.reset();
      // t = btg.grow_trek(last);
      // Completed a whole run, start over.
      if(!t){
	printf("all exhausted, running again\n");
	break;
      }

      if(optimum.update(*t->next_element, *btg.get_frontier())){
	std::cout << "took=" << optimum_timer.elapsed() / 1000 << "ms to find new optimum\n";
	optimum_timer.reset();
	printf("OUT BEGIN\n");
	t->next_element->print();
	printf("OUT END\n");
	if(last){
	  last->forward_dispose();
	  last = nullptr;
	}
      }else{
	double transition_prob = exp(-(btg.frontier->total_cost - last_state_cost) / temperature);
	if(uniform_real(GENERATOR) < transition_prob){
	  //printf("move from=%d to=%d\n", last_state_cost, btg.frontier->total_cost);
	  last_state_cost = btg.frontier->total_cost;
	  if(last){
	    last->forward_dispose();
	    last = nullptr;
	  }
	}else{
	  if(last){
	    btg.stepback_days(rollback_days);
	    t = btg.grow_trek(last);
	    last->forward_dispose();
	    last = nullptr;
	    //btg.regrow_pregrown(last, rollback_days);
	  }
	}
      }

      if(j >= TEMP_JUMP){
	temperature = temperature / float(2);
	printf("new temp=%f\n", temperature);
	if(temperature < 300){
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
