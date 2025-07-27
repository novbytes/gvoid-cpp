CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

build: src/main.cpp src/lexer.hpp src/parser.hpp src/tokens.hpp
	$(CXX) $(CXXFLAGS) -o gvoid src/main.cpp

clean:
	rm -f gvoid