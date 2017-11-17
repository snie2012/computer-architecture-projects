CC = g++ 

CPP_FILES := $(wildcard *.cc) 
OBJ_FILES := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o)))
OBJ := sim_cache
NO_WARNING = -w
STD_LIB = -std=c++03

all: $(SRC)
	$(CC) -o $(OBJ) $(STD_LIB) $(NO_WARNING) $(CPP_FILES)

run: all 
	./$(OBJ)

clean:
	@rm $(OBJ) 
