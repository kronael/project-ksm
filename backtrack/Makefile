CXXFLAGS = --std=gnu++11 -g -Wformat=0 -O0 -DENABLE_DEBUGGING -foptimize-sibling-calls
all: ts-forward-sa ts-backward-sa ts-nearest-first ts-forward-rsa
test-mock: *.hpp test.cpp backtrack.o spliceit.o
ts-forward-rsa: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-backward-sa: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-forward-sa: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
#	g++ $(CXXFLAGS) $< flight.o backtrack.o spliceit.o cityhash.o schedule.o
ts-nearest-shuffle: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-nearest-first: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
ts-plain-backtrack: flight.o backtrack.o spliceit.o cityhash.o schedule.o *.hpp
test-txt: backtrack.o spliceit.o cityhash.o *.hpp
test: *.hpp test.cpp backtrack.o
*.cpp: *.hpp
spliceit: spliceit.cpp
	g++ $(CXXFLAGS) $< -o spliceit
