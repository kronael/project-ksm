CXXFLAGS = --std=gnu++11 -g -Wformat=0 -O0 -DENABLE_DEBUGGING -foptimize-sibling-calls
PROD_CXXFLAGS = --std=gnu++11 -Wformat=0 -O3 -foptimize-sibling-calls
all: ts-forward-sa ts-backward-sa ts-nearest-first ts-forward-rsa
production: ts-thread.cpp flight.cpp backtrack.cpp spliceit.cpp cityhash.cpp schedule.cpp *.hpp
	g++ $(PROD_CXXFLAGS) -c flight.cpp -o flight.o
	g++ $(PROD_CXXFLAGS) -c backtrack.cpp -o backtrack.o
	g++ $(PROD_CXXFLAGS) -c spliceit.cpp -o spliceit.o
	g++ $(PROD_CXXFLAGS) -c cityhash.cpp -o cityhash.o
	g++ $(PROD_CXXFLAGS) -c schedule.cpp -o schedule.o
	g++ $(PROD_CXXFLAGS) -o $@ ts-thread.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o -pthread 
	strip ts-thread
test-mock: *.hpp test.cpp backtrack.o spliceit.o
ts-forward-greedy-sa: ts-forward-greedy-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o greedy.o *.hpp
	g++ $(CXXFLAGS) -o $@ ts-forward-greedy-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o greedy.o
ts-forward-exchanging-sa: ts-forward-exchanging-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
	g++ $(CXXFLAGS) -o $@ ts-forward-exchanging-sa.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o
ts-thread: ts-thread.cpp flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
	g++ $(CXXFLAGS) $^ -o $@ -pthread 
ts-forward-rsa: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-backward-sa: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-forward-sa: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
#	g++ $(CXXFLAGS) $< flight.o backtrack.o spliceit.o cityhash.o schedule.o
ts-nearest-shuffle: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-nearest-first: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-plain-backtrack: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
test-txt: backtrack.o spliceit.o cityhash.o *.hpp
test: test.cpp
*.cpp: *.hpp
spliceit: spliceit.cpp
	g++ $(CXXFLAGS) $< -o spliceit
.PHONY: production
