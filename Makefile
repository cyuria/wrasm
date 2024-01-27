
cc:=clang

rm:=rm

h:=h argp-standalone/include
src:=src
build:=build
bin:=bin

dirs:=$(h) $(src) $(build) $(bin)

argp-standalone-dir:=argp-standalone/build/src
argp-standalone:=$(argp-standalone-dir)/libargp-standalone.a

debug:=y
cflags:=-std=c17 -Wall
lflags:=-L$(argp-standalone-dir) -largp-standalone

ifeq ($(strip $(debug)),y)
	cflags+= -g
else
	cflags+= -O3
endif

output=$(bin)/wrasm
headers:=$(wildcard $(h)/*.h)
sources:=$(wildcard $(src)/*.c)
objects:=$(sources:$(src)/%.c=$(build)/%.o)

.PHONY: all test clean

all: $(dirs) $(output)

test: all
	@echo "tests have not yet been implemented"

$(output): $(objects) $(argp-standalone)
	$(cc) $(filter %.o,$^) $(lflags) -o $@

$(build)/%.o: $(src)/%.c $(headers)
	$(cc) -c $< $(addprefix -I,$(h)) -o $@ $(cflags)

$(dirs):
	mkdir -p $@

$(argp-standalone):
	(cd argp-standalone && cmake . -Bbuild && cmake --build build)

clean:
	$(rm) -rf $(build) $(bin)

