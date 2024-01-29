
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

.PHONY: all test clean

all: $(dirs) $(output)

test: $(output)
	@echo "tests have not yet been implemented"

$(output): $(objects)
	$(cc) $(filter %.o,$^) $(lflags) -o $@

$(build)/%.o: $(src)/%.c $(headers)
	$(cc) -c $< $(addprefix -I,$(h)) -o $@ $(cflags)

$(dirs):
	mkdir -p $@

clean:
	rm -rf $(build) $(bin)

