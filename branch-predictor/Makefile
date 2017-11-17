#!/usr/bin/make

# Compliler to use
CC = g++ 

# Conform to c++03 to meet EOS machine compiler requirements
STD_LIB = -std=c++03

# No warning
NO_WARNING = -w

# Source files and object files
CPP_FILES := $(wildcard *.cc) 
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))

# Executable
OBJ := sim_bp

# Build executable from source
all: $(SRC)
	$(CC) -o $(OBJ) $(STD_LIB) $(NO_WARNING) $(CPP_FILES)

# Run executable
run: all 
	./$(OBJ)

# Clean executable
clean:
	rm $(OBJ) 