CXXFLAGS = --std=c++11 -g -Wformat=0 -O0 -DENABLE_DEBUGGING
test-mock: *.hpp test.cpp backtrack.o spliceit.o
test-timing: test-timing.cpp backtrack.o spliceit.o cityhash.o *.hpp
test-txt: test-txt.cpp backtrack.o spliceit.o cityhash.o *.hpp
test: *.hpp test.cpp backtrack.o
spliceit: spliceit.cpp
	g++ $(CXXFLAGS) $< -o spliceit
