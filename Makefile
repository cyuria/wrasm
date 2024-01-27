
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
	libargpdir := argp-standalone/build/src
	libargpinclude := argp-standalone/include/argp-standalone
	lflags += -L$(lib) -largp-standalone
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

output := $(bin)/wrasm$(opext)
headers := $(wildcard $(h)/*.h)
sources := $(wildcard $(src)/*.c)
objects := $(sources:$(src)/%.c=$(build)/%.o)

.PHONY: all test clean

all: $(dirs) $(output)

test: $(output)
	@echo "tests have not yet been implemented"

$(output): $(objects) $(lib)/$(argp-standalone)
ifeq ($(OS),Windows_NT)
	-@echo "libargp-standalone $(lib)/$(argp-standalone)"
	ls lib
endif
	$(cc) $(filter %.o,$^) $(lflags) -o $@

$(build)/%.o: $(src)/%.c $(headers)
	$(cc) -c $< $(addprefix -I,$(h)) -o $@ $(cflags)

$(dirs):
	mkdir -p $@

$(lib)/$(libargp):
ifdef noargp
	(cd argp-standalone && cmake . -Bbuild -DCMAKE_BUILD_TYPE=RelWithDebInfo && cmake --build build)
	cp $$(find $(libargpdir) -type f -name $(libargp)) $(lib)/
endif

clean:
	rm -rf $(build) $(bin)

