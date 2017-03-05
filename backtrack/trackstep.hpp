#ifndef TRACKSTEP_H
#define TRACKSTEP_H

#include <stdio.h>

#include "debug.hpp"
#include "flight.hpp"


struct TrackStep
{
  int dept, dest, cost;
  FlightsGenerator flights_iter;
  TrackStep(int new_dept, int new_dest, int new_cost, FlightsGenerator& new_flights_iter) :
    dept(new_dept), dest(new_dest), cost(new_cost), flights_iter(new_flights_iter) {}
  // TrackStep(int new_dept, int new_dest, int new_cost, FlightsGenerator& new_flights_iter, FlightsGenerator& new_proposal_flights_iter) :
  //   dept(new_dept), dest(new_dest), cost(new_cost), flights_iter(new_flights_iter), proposal_flights_iter(new_flights_iter) {}
};


struct Track
{
  Track *next_element, *prev_element;
  int total_cost;
  TrackStep descr;
  Track(TrackStep& new_descr) :
    descr(new_descr), prev_element(nullptr), next_element(nullptr), total_cost(new_descr.cost) {}
  Track(TrackStep& new_descr, Track *prev) :
    descr(new_descr), prev_element(prev), next_element(nullptr){
    total_cost = new_descr.cost;
    if(prev)
      total_cost += prev->total_cost;
  }
  Track *enlarge(TrackStep& new_descr){
    next_element = new Track(new_descr, this);
    return next_element;
  }
  Track *shrink(){
    Track *new_track = prev_element;
    if(new_track)
      new_track->next_element = nullptr;
    delete this;
    return new_track;
  }
  bool empty(){
    return descr.dept == -1 && descr.dest == -1 && descr.cost == -1;
  }
  void print(){
    printf("%d -> %d $%d ($%d)\n", descr.dept, descr.dest, descr.cost, total_cost);
    if(next_element)
      next_element->print();
  }
  Track *duplicate(){
    return duplicate(new Track(descr));
  }
  Track *duplicate(Track *duped_frontier){ 
    DEBUG(duped_frontier->print());
    if(next_element)
      return next_element->duplicate(duped_frontier->enlarge(descr));
    return duped_frontier->enlarge(descr);
  }
  void dispose(){
    Track *new_track = shrink();
    if(new_track)
      new_track->dispose();
  }
  void forward_dispose(){
    if(next_element){
      next_element->forward_dispose();
    }else{
      dispose();
    }
  }
};

#endif

      // Flights empty_flights;
      // FlightsGenerator empty_gen(empty_flights);
      // TrackStep empty(-1, -1, -1, empty_gen);
      // return new Track(empty);
