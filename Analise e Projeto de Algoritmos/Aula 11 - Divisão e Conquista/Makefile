CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -pedantic -pthread

aula11: src/main_aula11.cpp src/*.hpp
	$(CXX) $(CXXFLAGS) -o aula11 src/main_aula11.cpp

todos: aula11
	./aula11 todos

clean:
	rm -f aula11 aula11.exe

.PHONY: todos clean
