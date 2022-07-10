CC := g++
RM := rm
CPP_FILES := $(foreach sdir,src/* src,$(wildcard $(sdir)/*.cpp))
OBJ_FILES := $(patsubst src/%.cpp,obj/%.o,$(CPP_FILES))
LD_FLAGS := -lpthread -lasound
CC_FLAGS := -D_REENTRANT -Wall -O3 -Isrc -Wno-strict-overflow

all: dragon-radar

dragon-radar: $(OBJ_FILES)
	$(CC) $(LD_FLAGS) -o $@ $^

obj/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CC_FLAGS) -c -o $@ $<
	
clean:
	$(RM) -r obj/*
	$(RM) dragon-radar
