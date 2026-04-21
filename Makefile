.PHONY: all clean disass_text

CC      ?= zig cc
CFLAGS  ?= -std=c23
SFLAGS  ?= -nostdlib
OBJCOPY ?= objcopy
OBJDUMP ?= objdump

BUILD := build

C_SRCS := $(wildcard *.c)
S_SRCS := $(wildcard *.s)

C_BINS := $(patsubst %.c,$(BUILD)/%,$(C_SRCS))
S_BINS := $(patsubst %.s,$(BUILD)/%,$(S_SRCS))

all: $(C_BINS) $(S_BINS) $(BUILD)/min_prog.text $(BUILD)/out

clean:
	rm -rf $(BUILD)

$(BUILD)/%: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $<

$(BUILD)/%: %.s
	@mkdir -p $(@D)
	$(CC) $(SFLAGS) -o $@ $<

$(BUILD)/min_prog.text: $(BUILD)/min_prog
	$(OBJCOPY) -O binary --only-section=.text $< $@

$(BUILD)/out: $(BUILD)/write_elf $(BUILD)/min_prog.text
	$< $@

disass_text: $(BUILD)/min_prog.text
	$(OBJDUMP) -D -b binary -m i386:x86-64 $<
