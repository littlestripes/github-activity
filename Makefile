CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++20 -I./include -g
LDFLAGS = `curl-config --libs`

SRC_DIR = src

SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJ = $(SRC:.cpp=.o)
EXEC = github-activity

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
