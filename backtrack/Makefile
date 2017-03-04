CXXFLAGS = --std=gnu++11 -g -Wformat=0 -O0 -DENABLE_DEBUGGING
test-mock: *.hpp test.cpp backtrack.o spliceit.o
test-shuffle: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
test-timing: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
test-txt: backtrack.o spliceit.o cityhash.o *.hpp
test: *.hpp test.cpp backtrack.o
spliceit: spliceit.cpp
	g++ $(CXXFLAGS) $< -o spliceit
