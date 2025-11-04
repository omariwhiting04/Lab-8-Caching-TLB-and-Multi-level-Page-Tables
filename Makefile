CXX = g++
CXXFLAGS = -std=c++11 -Wall

all: lab8 generate

lab8: main.cpp
	$(CXX) $(CXXFLAGS) -o lab8 main.cpp

generate: generate_addresses.cpp
	$(CXX) $(CXXFLAGS) -o generate_addresses generate_addresses.cpp

run: generate lab8
	./generate_addresses
	./lab8

clean:
	rm -f lab8 generate_addresses addresses.txt
