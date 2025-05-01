# src/Makefile

CXX = g++
CXXFLAGS = -std=c++17 -Wall -I. -I./packets
LDFLAGS = -lsqlite3 -lcrypto
TARGET = server

# Auto-detect all .cpp files inside src/ and ../packets/
SRC = $(wildcard *.cpp ../packets/*.cpp)
OBJ = $(SRC:.cpp=.o)

# Default target
all: $(TARGET)

# Link object files into final binary
$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Compile .cpp into .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean only this directory and ../packets/ object files and binary
clean:
	rm -f $(OBJ) $(TARGET)

# Clean all object files from subdirectories too (if any deeper objects exist)
clean-all:
	rm -f $(OBJ) $(TARGET) $(wildcard */*.o) $(wildcard ../packets/*.o)

# Remove extra dev files: dependency files, backups, logs
clean-up:
	rm -f $(wildcard *.d)
	rm -f $(wildcard *~)
	rm -f $(wildcard *.log)

# Force full rebuild
rebuild: clean-all all

.PHONY: all clean clean-all clean-up rebuild
