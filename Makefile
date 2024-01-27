
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
	noargp :=
	lflags := -L$(lib) -largp-standalone -Wl,-nodefaultlib:libcmt -D_DLL -lucrt
	libargp := argp-standalone.lib
	opext := .exe
endif
ifdef noargp
	h += argp-standalone/include/argp-standalone
endif
ifeq ($(OS),Darwin)
	lflags := -L/usr/local/opt/argp-standalone/lib -largp
	h += /usr/local/opt/argp-standalone/include
	libprefix := lib
	libext := .a
endif

argp-standalone-dir := argp-standalone/build/src
ifeq ($(OS),Darwin)
	argp-standalone := libargp-standalone.a
endif
ifeq ($(OS),Windows_NT)
	argp-standalone := argp-standalone.lib
endif

output := $(bin)/wrasm$(opext)
headers := $(wildcard $(h)/*.h)
sources := $(wildcard $(src)/*.c)
objects := $(sources:$(src)/%.c=$(build)/%.o)

.PHONY: all test clean

all: $(dirs) $(output)

test: all
	@echo "tests have not yet been implemented"

$(output): $(objects) $(lib)/$(argp-standalone)
	$(cc) $(filter %.o,$^) $(lflags) -o $@

$(build)/%.o: $(src)/%.c $(headers)
	$(cc) -c $< $(addprefix -I,$(h)) -o $@ $(cflags)

$(dirs):
	mkdir -p $@

$(lib)/$(argp-standalone):
ifneq ($(OS),Darwin)
	(cd argp-standalone && cmake . -Bbuild -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build)
	cp $$(find $(argp-standalone-dir) -type f -name $(argp-standalone)) $(lib)/
endif

clean:
	rm -rf $(build) $(bin)

