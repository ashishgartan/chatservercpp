# Simple Client Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -I. -I./packets
TARGET = client

# Sources and objects
SRC = $(wildcard *.cpp ../packets/*.cpp)
OBJ = $(SRC:.cpp=.o)

# Default target
all: $(TARGET)

# Link objects to create the executable
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

# Compile .cpp files to .o objects
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean generated object files and executable (current dir only)
clean:
	rm -f $(OBJ) $(TARGET)

# Remove all object files (*.o) and executables (including subdirectories like ../packets/)
clean-all:
	rm -f $(OBJ) $(TARGET) $(wildcard */*.o) $(wildcard ../packets/*.o)

# Clean up additional temporary files (like dependency files .d, etc.)
clean-up:
	rm -f $(wildcard *.d)  # Clean dependency files
	rm -f $(wildcard *~)   # Clean backup files (e.g., files ending with ~)
	rm -f $(wildcard *.log) # Clean log files

# Rebuild everything (force recompile)
rebuild: clean-all all

.PHONY: all clean clean-all clean-up rebuild
