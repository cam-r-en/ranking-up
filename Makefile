# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -O2

# Main program objects
MAIN_OBJS = main.o

# Submission objects (student code)
CORE_OBJS= \
	./Leaderboard.o \
	./Player.o \
	./PlayerStream.o

# Final object list
OBJS = $(MAIN_OBJS) $(CORE_OBJS)

# Program name
PROG ?= main

# Compile and link
$(PROG): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)

.cpp.o:
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Clean up
clean:
	rm -rf $(PROG) *.o $(SUBMISSION_DIR)/*.o $(TEST_DIR)/*.o

# Rebuild
rebuild: clean $(PROG)
