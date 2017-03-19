CXXFLAGS = --std=gnu++11 -g -Wformat=0 -DENABLE_DEBUGGING -foptimize-sibling-calls
PROD_CXXFLAGS = --std=gnu++11 -Wformat=0 -foptimize-sibling-calls
production: ts-thread.cpp flight.o backtrack.cpp spliceit.o cityhash.o schedule.o *.hpp
	g++ -O0 $(CXXFLAGS) -c backtrack.cpp -o backtrack.o
	g++ -O3 $(CXXFLAGS) -o ts-thread ts-thread.cpp flight.o backtrack.o cityhash.o spliceit.o schedule.o -pthread 
test-mock: *.hpp test.cpp backtrack.o spliceit.o
ts-forward-greedy-sa: ts-forward-greedy-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o greedy.o *.hpp
	g++ $(CXXFLAGS) -o $@ ts-forward-greedy-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o greedy.o
ts-forward-exchanging-sa: ts-forward-exchanging-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
	g++ $(CXXFLAGS) -o $@ ts-forward-exchanging-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o -pthread
ts-forward-rsa: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-backward-sa: flight.o backtrack.o spliceit.o cityhash.o *.hpp
	g++ $(CXXFLAGS) -O3 -o $@ ts-backward-sa.cpp flight.o backtrack.o spliceit.o cityhash.o -pthread
ts-forward-sa: ts-forward-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
	g++ $(CXXFLAGS) -O0 -o $@ ts-forward-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o -pthread
ts-forward-combining-sa: ts-forward-combining-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
	g++ $(CXXFLAGS) -O0 -o $@ ts-forward-combining-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o -pthread
ts-nearest-shuffle: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-nearest-first: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-plain-backtrack: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
test-txt: backtrack.o spliceit.o cityhash.o *.hpp
test: test.cpp
schedule.o: schedule.cpp schedule.hpp cityhash.hpp
	g++ -DCSV_IO_NO_THREAD $(CXXFLAGS) -O3 -c -o $@ $<
cityhash.o: cityhash.cpp cityhash.hpp
	g++ $(CXXFLAGS) -O3 -c -o $@ $<
spliceit.o: spliceit.cpp spliceit.hpp
	g++ $(CXXFLAGS) -O3 -c -o $@ $<
flight.o: flight.cpp flight.hpp
	g++ $(CXXFLAGS) -O3 -c -o $@ $<


*.cpp: *.hpp
spliceit: spliceit.cpp
	g++ $(CXXFLAGS) $< -o spliceit
sparse_generator: sparse_generator.cpp
.PHONY: production
