.PHONY: all clean disass_text

C_BINS := $(patsubst %.c,build/%,$(wildcard *.c))
S_BINS := $(patsubst %.s,build/%,$(wildcard *.s))

all: $(C_BINS) $(S_BINS) build/min_prog.text build/out

clean:
	-rm -rf build 2>/dev/null

build/%: %.c
	mkdir -p $(@D)
	zig cc -std=c23 -g -Og -o $@ $^

build/%: %.s
	mkdir -p $(@D)
	zig cc -nostdlib -o $@ $^

build/min_prog.text: build/min_prog
	objcopy -O binary --only-section=.text $^ $@

build/out: build/write_elf build/min_prog.text
	build/write_elf $@

disass_text: build/min_prog.text
	objdump -D -b binary -m i386:x86-64 $^
