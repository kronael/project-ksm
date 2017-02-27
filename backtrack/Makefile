CXXFLAGS = --std=c++11 -g -Wformat=0 -O0
test-mock: *.hpp test.cpp backtrack.o spliceit.o
test-txt: test-txt.cpp backtrack.o spliceit.o cityhash.o *.hpp
test: *.hpp test.cpp backtrack.o
spliceit: spliceit.cpp
	g++ $(CXXFLAGS) $< -o spliceit
