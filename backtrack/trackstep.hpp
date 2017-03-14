#ifndef TRACKSTEP_H
#define TRACKSTEP_H

#include <fstream>
#include <iostream>
#include <stdio.h>

#include "cityhash.hpp"
#include "debug.hpp"
#include "flight.hpp"

struct TrackStep
{
  unsigned int dept, dest, cost;
  FlightsGenerator flights_iter;
  TrackStep() :
    dept(0), dest(0), cost(0) {}
  TrackStep(unsigned int new_dept, unsigned int new_dest, unsigned int new_cost, FlightsGenerator& new_flights_iter) :
    dept(new_dept), dest(new_dest), cost(new_cost), flights_iter(new_flights_iter) {}
};


struct Track
{
  Track *next_element, *prev_element;
  int total_cost;
  TrackStep descr;
  Track() :
    prev_element(nullptr), next_element(nullptr), total_cost(0) {}
  Track(TrackStep& new_descr) :
    descr(new_descr), prev_element(nullptr), next_element(nullptr), total_cost(new_descr.cost) {}
  Track(TrackStep& new_descr, Track *prev) :
    descr(new_descr), prev_element(prev), next_element(nullptr){
    total_cost = new_descr.cost;
    if(prev)
      total_cost += prev->total_cost;
  }

  // Enlarges the track by including copy of the new_descr description
  // as the successor of the current track step.  Returns pointer to
  // the frontier of the enlarged track.
  Track *enlarge(TrackStep&& new_descr){
    next_element = new Track(new_descr, this);
    return next_element;
  }
  Track *enlarge(TrackStep& new_descr){
    next_element = new Track(new_descr, this);
    return next_element;
  }

  // Deletes the current track step, disposes of it and returns
  // pointer to the frontier of the shrinked track.
  Track *shrink(){
    Track *new_track = prev_element;
    if(new_track)
      new_track->next_element = nullptr;
    delete this;
    return new_track;
  }

  // Connects the tail to current track to form a new one.  Disposes
  // of the old tail.  Return pointer to the frontier of the new track.
  Track *connect(Track *tail){
    Track *old_next_element = next_element;
    if(next_element)
      old_next_element->forward_dispose();
    next_element = tail;
    tail->prev_element = this;
    if(descr.dest != tail->descr.dept){
      printf("FATAL: %d -> %d != %d\n", descr.dept, descr.dest, tail->descr.dept);
      prev_element->print();
    }
    return frontier();
  }

  // Returns true if the track object represents and empty track.
  bool empty(){
    return descr.dept == -1 && descr.dest == -1 && descr.cost == -1;
  }

  // Prints the track in forward order until the end.  I.E. prints the
  // current track step and all successive steps in the track.
  void print(){
    printf("%s -> %s $%d ($%d)\n", hashtag_out(descr.dept).c_str(), hashtag_out(descr.dest).c_str(), descr.cost, total_cost);
    if(next_element)
      next_element->print();
  }

  // Prints the track in forward order until the end.  I.E. prints the
  // current track step and all successive steps in the track.
  void print_terse(){
    printf("%s -> %s $%4d", hashtag_out(descr.dept).c_str(), hashtag_out(descr.dest).c_str(), descr.cost);
    if(next_element){
      printf(", ");
      next_element->print_terse();
    }else{
      printf("\n");
    }
  }

  // Prints the track in forward order until the end.  I.E. prints the
  // current track step and all successive steps in the track.
  void system_print(std::ostream& output = std::cout, int day = 0){
    output << hashtag_out(descr.dept) << " " << hashtag_out(descr.dest) << " " << day << " " << descr.cost << std::endl;
    if(next_element)
      next_element->system_print(output, day + 1);
  }

  // Same as system_print, but reverse destination and start.
  void reverse_system_print(std::ostream& output = std::cout, int day = 0){
    if(output == std::cout) printf("R ");
    output << hashtag_out(descr.dest) << " " << hashtag_out(descr.dept) << " " << day << " " << descr.cost << std::endl;
    if(prev_element) if(prev_element->prev_element) if(prev_element->prev_element->prev_element)
	prev_element->reverse_system_print(output, day + 1);
  }

  // Creates a copy of the track (current step and successive steps).
  // Returns pointer to the frontier of the newly created track.
  Track *duplicate(){
    return duplicate(new Track(descr));
  }

  // Connects the copy of the current track step to follow
  // duped_frontier.  Returns pointer to the frontier of the new
  // track.  This is a pointer to a duplicated track.
  Track *duplicate(Track *duped_frontier){ 
    DEBUG(duped_frontier->print());
    if(next_element)
      return next_element->duplicate(duped_frontier->enlarge(descr));
    return duped_frontier->enlarge(descr);
  }

  // Disposes of the track before current position.  Propagates from frontier backwards.
  void dispose(){
    Track *new_track = shrink();
    if(new_track)
      new_track->dispose();
  }

  // Disconnect the track and then disposes of the connected track
  // starting with this track step, i.e.  first searches forward to
  // find the frontier and then calls dispose().
  void forward_dispose(){
    disconnect();
    if(next_element){
      next_element->forward_dispose();
    }else{
      dispose();
    }
  }

  // Disconnects current track from the track (prev_element).
  void disconnect(){
    if(prev_element)
      prev_element->next_element = nullptr;
    prev_element = nullptr;
  }

  // Returns pinter to the last step of track.
  Track *frontier(){
    Track *i;
    for(i = this; i->next_element; i = i->next_element);
    return i;
  }
  Track *start(){
    Track *i;
    for(i = this; i->prev_element; i = i->prev_element);
    return i->next_element;
  }

  // Recalculate total_cost of the track from this track step forward,
  // assuming the previous step is either non-existent or has correct
  // total_cost.
  void fix_total_cost(){
    DEBUG(printf("fixing total cost %d -> %d ($%d)\n", descr.dept, descr.dest, descr.cost));
    if(prev_element){
      total_cost = prev_element->total_cost;
    }else{
      total_cost = 0;
    }
    total_cost += descr.cost;
    if(next_element)
      next_element->fix_total_cost();
  }

  // Validate the track ending at this and staring at start.  The
  // track is printed in reverse if reverse is true.
  int validate(Track& start, const char *input_file, bool reverse = false){
    std::ofstream output;
    output.open("flights.out");
    output << total_cost << std::endl;
    if(reverse){
      reverse_system_print(output);
    }else{
      start.system_print(output);
    }
    output.close();

    // Run the check script.
    char cmd[1024];
    snprintf(cmd, 1023, "python ../../travelling-salesman/verification_script/verify.py %s flights.out", input_file);
    printf("validating output by: %s\n", cmd);
    printf("current track: ");
    start.print_terse();
    if(system(cmd))
      throw "failed to validate";
  }
};

#endif
