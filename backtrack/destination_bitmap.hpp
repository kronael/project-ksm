#ifndef DESTINATION_BITMAP_H
#define DESTINATION_BITMAP_H

#include <stdio.h>
#include <vector>

#include "debug.hpp"

class DestinationBitmap
{
  unsigned int size;
  unsigned int visited_count;
  std::vector<bool> bitmap;
public:
  /*! Construct a bitmap of appropriate size.
   */
  DestinationBitmap(unsigned int new_size, unsigned int first) :
    size(new_size), visited_count(0){
    bitmap.assign(new_size, false);
    // bitmap[first] = true;
  }
  void visit(unsigned int city){
    DEBUG(printf("visiting city=%d\n", city));
    ++visited_count;
    bitmap[city] = true;
  }
  void unvisit(unsigned int city){
    DEBUG(printf("un-visiting city=%d\n", city));
    --visited_count;
    bitmap[city] = false;
  }
  bool visited(unsigned int city){
    return bitmap[city];
  }
  bool visited_all(){
    return visited_count == size;
  }
  bool almost_all_visited(){
    return visited_count == size - 1;
  }
};

#endif
