#include <chrono>
#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include <future>
#include <memory>

#include "backtrack.hpp"
#include "debug.hpp"
#include "schedule.hpp"
#include "trackstep.hpp"
#include "trekoptimum.hpp"
#include "types.hpp"

std::chrono::seconds TIME_TO_LIVE;

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



BackwardsSchedule backward_schedule;
TxtSchedule forward_schedule;
TxtSchedule greedy_forward_schedule;
std::vector<TrekOptimum> optima;
bool reverse[4] = {false, false, false, false};
bool times_up = false;
std::mutex yard_gate;
std::condition_variable graveyard;
unsigned int corpses_count = 0;
unsigned int starting_city;

// The parameters here are specified in the same order as in which
// they are read from the parameters list.
const int EACH_N = 10000;

const char* INPUT_FILE;

double FORWARD_INITIAL_TEMPERATURE;
unsigned int FORWARD_TEMP_JUMP;
unsigned int FORWARD_PARAM_SUCCESSES;
double FORWARD_ZERO_TEMP;
double FORWARD_PARAM_SUCCESS_PR;

double BACKWARDS_INITIAL_TEMPERATURE;
unsigned int BACKWARDS_TEMP_JUMP;
unsigned int BACKWARDS_PARAM_SUCCESSES;
double BACKWARDS_ZERO_TEMP;
double BACKWARDS_PARAM_SUCCESS_PR;

double EXCHANGE_PROBABILITY;

void execute_ts_backward_sorted_sa(unsigned int id)
{
  std::default_random_engine GENERATOR(1987 + (1074 * id) % 39);
  const int TEMP_JUMP = BACKWARDS_TEMP_JUMP;
  const int PARAM_SUCCESSES = BACKWARDS_PARAM_SUCCESSES;
  const double PARAM_SUCCESS_PR = BACKWARDS_PARAM_SUCCESS_PR;
  double temperature = BACKWARDS_INITIAL_TEMPERATURE;
  int last_state_cost = MAX_INT;

  bool we_re_done = false;
  Timer timer;
  Timer optimum_timer;
  Track *t, *last, *current;
  int i, j;
  int times[EACH_N];
  int rollback_days;
  std::negative_binomial_distribution<int> days_distribution(2, 0.2);
  std::uniform_real_distribution<double> uniform_real(0, 1);

  std::cout << "worker id=" << id << " on thread=" << std::this_thread::get_id() << std::endl;

  while(1){
    BacktrackingWayGenerator btg(backward_schedule, starting_city);
    t = btg.grow_trek();
    current = btg.get_start()->duplicate();
    do{
      // Do the threading work.
      if(times_up || we_re_done){
	{
	  std::cout << "times up, dying" << std::endl;
	  std::lock_guard<std::mutex> lock(yard_gate);
	  ++corpses_count;
	}
	graveyard.notify_all();
	return;
      }

      if(t){
	rollback_days = days_distribution(GENERATOR);
	last = btg.stepback_days(rollback_days);
      }
      timer.reset();
      t = btg.grow_trek(last, true);
      // Completed a whole run, start over.
      if(!t){
	DEBUG(printf("all exhausted, running again\n"));
	break;
      }

      if(optima[id].update(*t, btg.get_frontier()->total_cost)){
	std::cout << "took=" << optimum_timer.elapsed() / 1000 << "ms to find new BS optimum cost=" << btg.get_frontier()->total_cost << std::endl;
	optimum_timer.reset();
      }else{
	double transition_prob = exp(-(btg.frontier->total_cost - last_state_cost) / temperature);
	if(uniform_real(GENERATOR) < transition_prob){
	  last_state_cost = btg.frontier->total_cost;
	  current->forward_dispose();
	  current = btg.get_start()->duplicate();
	}else{
	  btg.exchange_track(current->start()->duplicate());
	}
      }

      if(j >= TEMP_JUMP){
	temperature = temperature / float(2);
	printf("new temp=%f\n", temperature);
	if(temperature < FORWARD_ZERO_TEMP){
	  printf("zero temperature: we're done\n");
	  we_re_done = true;
	}
	j = 0;
      }
      ++j;
    } while(t);
  }
}


void execute_ts_forward_sorted_sa_worker(unsigned int id, TxtSchedule& schedule)
{
  std::default_random_engine GENERATOR(1987 + (1074 * id) % 39);
  const int TEMP_JUMP = FORWARD_TEMP_JUMP;
  const int PARAM_SUCCESSES = FORWARD_PARAM_SUCCESSES;
  const double PARAM_SUCCESS_PR = FORWARD_PARAM_SUCCESS_PR;
  double temperature = FORWARD_INITIAL_TEMPERATURE;
  int last_state_cost = MAX_INT;

  bool we_re_done = false;
  Timer timer;
  Timer optimum_timer;
  Track *t, *last;
  Track *current;
  int i, j;
  int times[EACH_N];
  int rollback_days;
  std::negative_binomial_distribution<int> days_distribution(PARAM_SUCCESSES, PARAM_SUCCESS_PR);
  std::uniform_real_distribution<double> uniform_real(0, 1);

  std::cout << "worker id=" << id << " on thread=" << std::this_thread::get_id() << std::endl;

  while(1){
   BacktrackingWayGenerator btg(schedule, starting_city);
    t = btg.grow_trek();
    current = btg.get_start()->duplicate();
    do{
      // Do the threading work.
      if(times_up || we_re_done){
	{
	  std::cout << "times up, dying" << std::endl;
	  std::lock_guard<std::mutex> lock(yard_gate);
	  ++corpses_count;
	}
	graveyard.notify_all();
	return;
      }

      if(t){
	rollback_days = days_distribution(GENERATOR);
	last = btg.stepback_days(rollback_days);
      }
      timer.reset();
      t = btg.grow_trek(nullptr, true);
      // Completed a whole run, start over.
      if(!t){
	DEBUG(printf("all exhausted, running again\n"));
	break;
      }


      if(optima[id].update(*t, btg.get_frontier()->total_cost)){
	std::cout << "took=" << optimum_timer.elapsed() / 1000 << "ms to find new FS optimum cost=" << btg.get_frontier()->total_cost << std::endl;
	optimum_timer.reset();
      }else{
	double transition_prob = exp(-(btg.frontier->total_cost - last_state_cost) / temperature);
	if(uniform_real(GENERATOR) < transition_prob){
	  last_state_cost = btg.frontier->total_cost;
	  current->forward_dispose();
	  current = btg.get_start()->duplicate();
	}else{
	  btg.exchange_track(current->start()->duplicate());
	}
      }

      if(j >= TEMP_JUMP){
	temperature = temperature / float(2);
	printf("new temp=%f\n", temperature);
	if(temperature < BACKWARDS_ZERO_TEMP){
	  printf("zero temperature: we're done\n");
	  we_re_done = true;
	}
	j = 0;
      }
      ++j;
    } while(t);
  }
}


void execute_ts_forward_exchanging_sa_worker(unsigned int id, Schedule& schedule)
{
  std::default_random_engine GENERATOR(1987 + (1074 * id) % 39);
  unsigned int TEMP_JUMP = FORWARD_TEMP_JUMP;
  const int PARAM_SUCCESSES = FORWARD_PARAM_SUCCESSES;
  const double PARAM_SUCCESS_PR = FORWARD_PARAM_SUCCESS_PR;
  double temperature = FORWARD_INITIAL_TEMPERATURE;
  unsigned int last_state_cost = MAX_INT;
  const double FORWARD_EX_ZERO_TEMP = FORWARD_ZERO_TEMP;

  bool we_re_done = false;
  Timer timer;
  Timer optimum_timer;
  Track *t, *last, *current;
  unsigned int i;
  unsigned int j = TEMP_JUMP;
  int times[EACH_N];
  unsigned int rollback_days;
  double temp_dec_factor = 4;
  unsigned int param_successes = int(PARAM_SUCCESSES * forward_schedule.schedule_days / 70);
  std::negative_binomial_distribution<int> days_distribution(param_successes, PARAM_SUCCESS_PR);
  std::uniform_int_distribution<int> uniform_days(2, forward_schedule.schedule_days - 1);
  std::uniform_real_distribution<double> uniform_real(0, 1);

  std::cout << "worker id=" << id << " on thread=" << std::this_thread::get_id() << std::endl;

  bool regrown_flights;
  bool switched_flights;
  while(true){
    BacktrackingWayGenerator btg(schedule, starting_city);
    t = btg.grow_trek();
    unsigned int starting_cost = btg.frontier->total_cost / 100;
    current = btg.get_start()->duplicate();
    do{
      // Do the threading work.
      if(times_up || we_re_done){
	{
	  std::cout << "times up, dying" << std::endl;
	  std::lock_guard<std::mutex> lock(yard_gate);
	  ++corpses_count;
	}
	graveyard.notify_all();
	return;
      }

      regrown_flights = false;
      switched_flights = false;

      if(t){
	if(uniform_real(GENERATOR) < 1 - EXCHANGE_PROBABILITY){
	  if(uniform_real(GENERATOR) < double(forward_schedule.schedule_days) / 100 * 0.45){
	    rollback_days = days_distribution(GENERATOR);
	  }else{
	    rollback_days = uniform_days(GENERATOR);
	  }
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
	  btg.get_start()->fix_total_cost();
	  t = btg.get_start();
	}
      }
      timer.reset();
      // Completed a whole run, start over.
      if(!t){
	DEBUG(printf("all exhausted, running again\n"));
	break;
      }

      if(optima[id].update(*t, btg.get_frontier()->total_cost)){
	if(forward_schedule.schedule_days <= 100){
	  if(j <= 2 * TEMP_JUMP)
	    j += TEMP_JUMP / 4;
	}else{
	  j = TEMP_JUMP;
	}
	// Advance into the new optimum.
	std::cout << "took=" << optimum_timer.elapsed() / 1000 << "ms to find new FE optimum cost=" << btg.get_frontier()->total_cost << std::endl;
	optimum_timer.reset();
	DEBUG(t->frontier()->validate(*btg.get_start()->next_element, INPUT_FILE));
	last_state_cost = btg.frontier->total_cost;
	current->forward_dispose();
	current = btg.get_start()->duplicate();
      }else{
	if(btg.frontier->total_cost < last_state_cost){
	  last_state_cost = btg.frontier->total_cost;
	  current->forward_dispose();
	  current = btg.get_start()->duplicate();
	}else{
	  // Explore non-optimal states.
	  double transition_prob = exp(-(btg.frontier->total_cost - double(last_state_cost)) / double(starting_cost) / temperature);
	  if(uniform_real(GENERATOR) < transition_prob){
	    if(i++ >= EACH_N){
	      i = 0;
	      printf("move from=%d to=%d at p=%f\n", last_state_cost, btg.frontier->total_cost, transition_prob);
	    }
	    last_state_cost = btg.frontier->total_cost;
	    current->forward_dispose();
	    current = btg.get_start()->duplicate();
	  }else{
	    DEBUG(printf("not stepping into from=%d to=%d\n", last_state_cost, btg.frontier->total_cost));
	    if(regrown_flights){
	      btg.exchange_track(current->start()->duplicate());
	    }
	    if(switched_flights){
	      if(!btg.switch_flight(rollback_days)){
		throw new FlightExchangeError();
		printf("failed to switch back\n");
	      }
	      btg.get_start()->fix_total_cost();
	    }
	  }
	}
      }

      //if(j % (TEMP_JUMP / 2) == 0){
      if(j <= 0){
	btg.exchange_track(optima[id].minimum->start()->duplicate());
	btg.get_start()->fix_total_cost();
	last_state_cost = btg.frontier->total_cost;
	current->forward_dispose();
	current = btg.get_start()->duplicate();
	printf("resetting cost=%d\n", last_state_cost);
      }

      if(j <= 0){
	if(temperature > 0.5)
	  temperature /= temp_dec_factor;
	// 20 - 0.5
	// 30 - 0.2
	if(temperature < 0.5 && forward_schedule.schedule_days < 70){
	  btg = BacktrackingWayGenerator(forward_schedule, starting_city);
	  t = btg.grow_trek();
	  starting_cost = btg.frontier->total_cost / 100;
	  current = btg.get_start()->duplicate();
	  j = TEMP_JUMP;
	  temperature = FORWARD_INITIAL_TEMPERATURE;
	}
	temp_dec_factor = 1.5;
	printf("new temp=%f cost=%d\n", temperature, last_state_cost);
	if(temperature < FORWARD_EX_ZERO_TEMP){
	  printf("zero temperature: we're done\n");
	  we_re_done = true;
	}
	j = TEMP_JUMP;
      }
      --j;
    } while(t);
  }
}


void execute_ts_be_sa(unsigned int id){
  execute_ts_forward_exchanging_sa_worker(id, backward_schedule);
}


void execute_ts_fe_sa(unsigned int id){
  execute_ts_forward_exchanging_sa_worker(id, forward_schedule);
}


void execute_ts_backwards_exchanging_sa(unsigned int id)
{
  std::default_random_engine GENERATOR(1987 + (1074 * id) % 39);
  const int TEMP_JUMP = BACKWARDS_TEMP_JUMP;
  const int PARAM_SUCCESSES = BACKWARDS_PARAM_SUCCESSES;
  const double PARAM_SUCCESS_PR = BACKWARDS_PARAM_SUCCESS_PR;
  double temperature = BACKWARDS_INITIAL_TEMPERATURE;
  int last_state_cost = MAX_INT;
  const double BACKWARDS_EX_ZERO_TEMP = BACKWARDS_ZERO_TEMP;

  bool we_re_done = false;
  Timer timer;
  Timer optimum_timer;
  Track *t, *last, *current;
  unsigned int i, j;
  int times[EACH_N];
  unsigned int rollback_days;
  std::negative_binomial_distribution<int> days_distribution(PARAM_SUCCESSES, PARAM_SUCCESS_PR);
  std::uniform_int_distribution<int> uniform_days(2, forward_schedule.schedule_days - 1);
  std::uniform_real_distribution<double> uniform_real(0, 1);

  std::cout << "worker id=" << id << " on thread=" << std::this_thread::get_id() << std::endl;

  bool regrown_flights;
  bool switched_flights;
  while(true){
    BacktrackingWayGenerator btg(backward_schedule, starting_city);
    t = btg.grow_trek();
    current = btg.get_start()->duplicate();
    do{
      // Do the threading work.
      if(times_up || we_re_done){
	{
	  std::cout << "times up, dying" << std::endl;
	  std::lock_guard<std::mutex> lock(yard_gate);
	  ++corpses_count;
	}
	graveyard.notify_all();
	return;
      }

      regrown_flights = false;
      switched_flights = false;

      if(t){
	if(uniform_real(GENERATOR) < 1 - EXCHANGE_PROBABILITY){
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

      if(optima[id].update(*t, btg.get_frontier()->total_cost)){
	// Advance into the new optimum.
	std::cout << "took=" << optimum_timer.elapsed() / 1000 << "ms to find new BE optimum cost=" << btg.get_frontier()->total_cost << std::endl;
	optimum_timer.reset();
	DEBUG(t->frontier()->validate(*btg.get_start()->next_element, INPUT_FILE));
      }else{
	// Explore non-optimal states.
	double transition_prob = exp(-(btg.frontier->total_cost - last_state_cost) / temperature);
	if(uniform_real(GENERATOR) < transition_prob){
	  DEBUG(printf("move from=%d to=%d\n", last_state_cost, btg.frontier->total_cost));
	  last_state_cost = btg.frontier->total_cost;
	  current->forward_dispose();
	  current = btg.get_start()->duplicate();
	}else{
	  DEBUG(printf("not stepping into from=%d to=%d\n", last_state_cost, btg.frontier->total_cost));
	  if(regrown_flights){
	    btg.exchange_track(current->start()->duplicate());
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
	printf("new temp=%f\n", temperature);
	//	btg.exchange_track(optima[id].minimum->start()->duplicate());
	if(temperature < BACKWARDS_EX_ZERO_TEMP){
	  printf("zero temperature: we're done\n");
	  we_re_done = true;
	}
	j = 0;
      }
      ++j;
    } while(t);
  }
}



// Sleeps a specified time, then notifies worker thread to stop
// working, waits for them to stop, calculates the best track, outputs
// and dies.
void grim_reap(unsigned int worker_count)
{
  std::this_thread::sleep_for(TIME_TO_LIVE);
  times_up = true;
  std::unique_lock<std::mutex> lock(yard_gate);
  graveyard.wait(lock, [=]{return corpses_count == worker_count;});

  int current_min_cost = MAX_INT;
  Track *current_optimum;
  int optimum_i = 0;
  for(int i = 0; i < optima.size(); ++i)
    if(current_min_cost > optima[i].minimal_cost){
      current_min_cost = optima[i].minimal_cost;
      current_optimum = optima[i].minimum;
      optimum_i = i;
    }
  printf(">>> SYSTEM OUTPUT\n");
  printf("%d\n", current_min_cost);
  if(current_min_cost < MAX_INT && current_optimum)
    if(reverse[optimum_i]){
      current_optimum->frontier()->reverse_system_print();
    }else{
      current_optimum->start()->next_element->system_print();
    }

  
  std::ofstream output;
  output.open("ts-thread.out");
  output << current_min_cost << std::endl;
  if(current_min_cost < MAX_INT && current_optimum)
    if(reverse[optimum_i]){
      current_optimum->frontier()->reverse_system_print(output);
    }else{
      current_optimum->start()->next_element->system_print(output);
    }
  output.close();

  // Run the check script.
  char cmd[1024];
  snprintf(cmd, 1023, "python ../../travelling-salesman/verification_script/verify.py %s ts-thread.out", INPUT_FILE);
  printf("validating output by: %s\n", cmd);
  system(cmd);

  return;
}


void read_in_params(int argc, char **argv)
{
  if(argc < 1){
    printf("specify input file");
    exit(1);
  }

  TIME_TO_LIVE = std::chrono::seconds(atoi(getenv("TIME_TO_LIVE")));

  FORWARD_INITIAL_TEMPERATURE = atof(getenv("FORWARD_INITIAL_TEMPERATURE"));
  FORWARD_TEMP_JUMP = atoi(getenv("FORWARD_TEMP_JUMP"));
  FORWARD_INITIAL_TEMPERATURE = atoi(getenv("FORWARD_INITIAL_TEMPERATURE"));
  FORWARD_PARAM_SUCCESSES = atoi(getenv("FORWARD_PARAM_SUCCESSES"));
  FORWARD_ZERO_TEMP = atoi(getenv("FORWARD_ZERO_TEMP"));
  FORWARD_PARAM_SUCCESS_PR = atof(getenv("FORWARD_PARAM_SUCCESS_PR"));

  BACKWARDS_INITIAL_TEMPERATURE = atof(getenv("BACKWARDS_INITIAL_TEMPERATURE"));
  BACKWARDS_TEMP_JUMP = atoi(getenv("BACKWARDS_TEMP_JUMP"));
  BACKWARDS_INITIAL_TEMPERATURE = atoi(getenv("BACKWARDS_INITIAL_TEMPERATURE"));
  BACKWARDS_PARAM_SUCCESSES = atoi(getenv("BACKWARDS_PARAM_SUCCESSES"));
  BACKWARDS_ZERO_TEMP = atoi(getenv("BACKWARDS_ZERO_TEMP"));
  BACKWARDS_PARAM_SUCCESS_PR = atof(getenv("BACKWARDS_PARAM_SUCCESS_PR"));

  EXCHANGE_PROBABILITY = atof(getenv("EXCHANGE_PROBABILITY"));

  INPUT_FILE = argv[1];

  char *debugging = getenv("DEBUG");
  if(debugging && (strcmp(debugging, "1") == 0))
    DEBUGGING_ENABLED = true;
}

int main(int argc, char **argv)
{
  if(argc < 1)
    exit(1);
  read_in_params(argc, argv);

  Timer timer;
  optima.push_back(TrekOptimum());
  optima.push_back(TrekOptimum());
  optima.push_back(TrekOptimum());
  optima.push_back(TrekOptimum());
  std::vector<std::thread*> workers;

  // Start the killer thread.
  workers.push_back(new std::thread(grim_reap, 2));

  // Start forward SA solver.
  starting_city = forward_schedule.load_flights_from_file(stdin);
  forward_schedule.sort_flights();
  std::cout << "took=" << timer.elapsed() / 1e6 << "s to load data" << std::endl;
  timer.reset();

  //workers.push_back(new std::thread(execute_ts_forward_sorted_sa, 1));
  //reverse[1] = false;

  //workers.push_back(new std::thread(execute_ts_forward_exchanging_sa, 2));
  //reverse[2] = false;

  // Start backwards SA solver.
  forward_schedule.sort_backwards_flights();
  backward_schedule = forward_schedule;
  std::cout << "took=" << timer.elapsed() / 1e6 << "s to sort backwards schedule" << std::endl;
  timer.reset();

  execute_ts_be_sa(0);

  // workers.push_back(new std::thread(execute_ts_backward_sorted_sa, 0));
  reverse[0] = true;

  //workers.push_back(new std::thread(execute_ts_backwards_exchanging_sa, 3));
  //reverse[3] = true;

  for(auto i = std::begin(workers); i != std::end(workers); ++i)
    (*i)->join();
}
