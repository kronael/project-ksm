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

  // Enlarges the track by including copy of the new_descr description
  // as the successor of the current track step.  Returns pointer to
  // the frontier of the enlarged track.
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
    printf("%d -> %d $%d ($%d)\n", descr.dept, descr.dest, descr.cost, total_cost);
    if(next_element)
      next_element->print();
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
};

#endif
