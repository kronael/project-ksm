#ifndef DESTINATION_BITMAP_H
#define DESTINATION_BITMAP_H

#include <stdio.h>
#include <vector>
#include <assert.h>

#include "debug.hpp"
#include "cityhash.hpp"


class DestinationBitmap
{
  unsigned int total_count;
  std::vector<bool> bitmap;
public:
  unsigned int visited_count;
  /*! Construct a bitmap of appropriate size.
   */
  DestinationBitmap(unsigned int new_total_count, unsigned int first) :
    total_count(new_total_count), visited_count(0){
    bitmap.assign(HASH_SIZE, false);
    // bitmap[first] = true;
  }
  void visit(unsigned int city){
    DEBUG(printf("visiting city=%d visited=%d\n", city, visited_count));
    DEBUG(assert(!bitmap[city]));
    ++visited_count;
    bitmap[city] = true;
  }
  void unvisit(unsigned int city){
    DEBUG(printf("un-visiting city=%d visited=%d\n", city, visited_count));
    DEBUG(assert(bitmap[city]));
    DEBUG(assert(visited_count > 0));
    --visited_count;
    bitmap[city] = false;
  }
  bool visited(unsigned int city){
    return bitmap[city];
  }
  bool visited_all(){
    DEBUG(printf("visited_count=%d total_count=%d\n", visited_count, total_count));
    return visited_count == total_count;
  }
  bool almost_all_visited(){
    return visited_count == total_count - 1;
  }
};

#endif
