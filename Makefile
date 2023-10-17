CC=gcc
COMMON_FLAGS=-DFREESTANDING_TRULY -Wall -Wpedantic -Wextra

CCF=-O2 -std=c89 -ffreestanding $(COMMON_FLAGS)
LDF=
TEST_CCF=-DDEBUG_TEST -O0 -g -std=c99 $(COMMON_FLAGS)
TEST_LDF=
LIBS=



OUTPUT_NAME=stdlib.a
EXEC_FMT=
ifeq ($(OS),Windows_NT)
	EXEC_FMT=.exe
endif


SRCS=$(wildcard src/*.c)
OBJS=$(patsubst src/%.c,obj/%.o,$(SRCS))
OUTPUT=$(OUTPUT_NAME)
OUT_DIRS=obj bin


.PHONY:all clean

all:library test

$(OUT_DIRS):
	mkdir $@

test:clean $(OUT_DIRS)
	$(foreach i_src,$(SRCS),\
		$(CC) $(TEST_CCF) $(TEST_LDF) \
			-o $(patsubst src/%.c,bin/%$(EXEC_FMT),$(i_src)) \
			$(i_src) \
			$(LIBS)\
	)

library:$(OUT_DIRS) $(OBJS)
	@echo "TODO: library target"


obj/%.o:src/%.c 
	$(CC) $(CCF) -c $^ -o $@ 

clean:
	rm -f obj/*
	rm -f bin/*
	rm -rf $(OUT_DIRS)
