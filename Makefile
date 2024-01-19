
cc:=clang
emu:=qemu-system-riscv64

h:=h
src:=src
build:=build
bin:=bin

dirs:=$(h) $(src) $(build) $(bin)

debug:=y
cflags:=-std=c17 -Wall
lflags:=

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

$(output): $(objects)
	$(cc) $^ $(lflags) -o $@

$(build)/%.o: $(src)/%.c $(headers)
	$(cc) -c $< $(addprefix -I,$(h)) -o $@ $(cflags)

$(dirs):
	mkdir -p $@

clean:
	rm -rf $(build) $(bin)

