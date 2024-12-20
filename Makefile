CXX = g++
CXXFLAGS = -std=c++20 -I./include -g

SRC = src/main.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = github-activity

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
