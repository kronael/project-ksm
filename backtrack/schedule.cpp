#include "schedule.hpp"

#include "csv.h"
#include "cityhash.hpp"

// Returns starting city.
int TxtSchedule::load_flights_from_file(FILE* infile){
  char *line = nullptr;
  size_t len = 0;
  ssize_t read;
  int nfields;

  // First line is special (starting city, discard here).
  if((read = getline(&line, &len, infile)) == -1){
    printf("failed to read first line containing starting city\n");
    exit(EXIT_FAILURE);
  }
  char *start_chopped;
  DEBUG(printf("starting city=%s\n", line));
  chop(line, &start_chopped);
  DEBUG(printf("starting city=%s\n", start_chopped));
  int starting_city = hashtag_in(start_chopped);
  DEBUG(printf("starting city=%d\n", starting_city));
  free(start_chopped);
  free(line);

  io::CSVReader<4, io::trim_chars<>, io::no_quote_escape<' '>> in("dummy", infile);
  in.set_header("dept", "dest", "day", "cost");
  char dept[4];
  char *dept_ptr = (char*)dept;
  char dest[4];
  char *dest_ptr = (char*)dest;
  unsigned int day;
  unsigned int cost;
  unsigned int i = 0;
  while(in.read_row(dept_ptr, dest_ptr, day, cost)){
    add_flight(hashtag_in(dept_ptr), hashtag_in(dest_ptr), day, cost);
  }

  // Reverse the backwards schedule for the days to match.
  std::reverse(backwards_schedule.begin(), backwards_schedule.end());

  return starting_city;
}
