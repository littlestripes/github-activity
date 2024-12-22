CXX = g++
CXXFLAGS = -std=c++20 -I./include -g
LDFLAGS = `pkg-config --libs --cflags fmt` `curl-config --libs`

SRC = src/main.cpp
OBJ = $(SRC:.cpp=.o)
EXEC = github-activity

$(EXEC): $(OBJ)
	$(CXX) $(OBJ) -o $(EXEC) $(LDFLAGS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
