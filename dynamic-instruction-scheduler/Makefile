#!/usr/bin/make

# Compliler to use
CC = g++-4.8

# Conform to c++03 to meet EOS machine compiler requirements
STD_LIB = -std=c++03

# Combine all options
OPT := $(STD_LIB) $(NO_WARNING)

# Source files and object files
CPP_FILES := $(wildcard *.cc)
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))

# Executable
OBJ := sim_ds

# Build executable from source
all: $(SRC)
	$(CC) -o $(OBJ) $(OPT) $(CPP_FILES)


# Clean executable
clean:
	rm $(OBJ) 
