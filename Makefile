
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
	noargp := y
	lflags += -Wl,-nodefaultlib:libcmt -D_DLL -lucrt
	opext := .exe
endif
ifeq ($(OS),Darwin)
	libargppath := /usr/local/opt/argp-standalone/lib
	libargpinclude := /usr/local/opt/argp-standalone/include
	sysargp := y
endif

ifdef noargp
	libargpinclude := argp-standalone/include/argp-standalone
	ldflags += -L$(lib) -largp-standalone
	ifeq ($(OS),Windows_NT)
		libargp := argp-standalone.lib
	else
		libargp := libargp-standalone.a
	endif
endif

ifdef libargppath
	lflags += -L$(libargppath)
	uselibargp := y
endif

ifdef uselibargp
	lflags += -largp
endif

ifdef libargpinclude
	h += $(libargpinclude)
endif

argp-standalone-dir := argp-standalone/build/src

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
ifdef noargp
	(cd argp-standalone && cmake . -Bbuild -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build)
	cp $$(find $(argp-standalone-dir) -type f -name $(argp-standalone)) $(lib)/
endif

clean:
	rm -rf $(build) $(bin)

