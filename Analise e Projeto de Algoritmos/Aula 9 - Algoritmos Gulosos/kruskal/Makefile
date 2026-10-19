CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic

kruskal: src/main.cpp src/*.hpp
	$(CXX) $(CXXFLAGS) -o kruskal src/main.cpp

todos: kruskal
	./kruskal todos

clean:
	rm -f kruskal kruskal.exe

.PHONY: todos clean
