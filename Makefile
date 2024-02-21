
ifndef OS
	OS := $(shell uname -s)
endif

cc := clang

h := h
src := src
lib := lib
build := build
bin := bin

dirs := $(h) $(lib) $(src) $(build) $(bin)

cflags := -std=c17 -Wall
lflags :=

opext :=

ifndef release
	cflags += -g
else
	cflags += -O3
endif

ifeq ($(OS),Windows_NT)
	cflags += -D_CRT_SECURE_NO_WARNINGS
else
	lflags += -lm
endif

output := $(bin)/wrasm$(opext)
headers := $(wildcard $(h)/*.h)
sources := $(wildcard $(src)/*.c)
objects := $(sources:$(src)/%.c=$(build)/%.o)

.PHONY: all test clean wordcount

all: $(dirs) $(output)

test: $(output)
	$(MAKE) -C test

$(output): $(objects)
	$(cc) $(filter %.o,$^) $(lflags) -o $@

$(build)/%.o: $(src)/%.c $(headers)
	$(cc) -c $< $(addprefix -I,$(h)) -o $@ $(cflags)

$(dirs):
	mkdir -p $@

clean:
	rm -rf $(build) $(bin)

wordcount:
	wc $(filter-out src/argtable3.c,$(sources)) $(filter-out h/argtable3.h,$(headers)) Makefile test/Makefile $(wildcard test/*.S)

