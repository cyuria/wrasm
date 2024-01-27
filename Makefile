
ifndef OS
	OS := $(shell uname -s)
endif

cc := clang

h := h argp-standalone/include
src := src
lib := lib
build := build
bin := bin

dirs := $(h) $(lib) $(src) $(build) $(bin)

debug := y
cflags := -std=c17 -Wall
lflags := -L$(lib) -largp-standalone

ifeq ($(strip $(debug)),y)
	cflags += -g
else
	cflags += -O3
endif

ifeq ($(OS),Windows_NT)
	libprefix :=
	libext := .lib
	opext := .exe
	lflags += -Wl,-nodefaultlib:libcmt -D_DLL -lucrt
endif
ifeq ($(OS),Linux)
	libprefix := lib
	libext := .a
	opext :=
endif
ifeq ($(OS),Darwin)
	libprefix := lib
	libext := .a
	opext :=
endif

argp-standalone-dir := argp-standalone/build/src
argp-standalone := $(libprefix)argp-standalone$(libext)

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
	(cd argp-standalone && cmake . -Bbuild -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang && cmake --build build)
	cp $$(find $(argp-standalone-dir) -type f -name $(argp-standalone)) $(lib)/

clean:
	rm -rf $(build) $(bin)

