# No BS(S)!

This repo builds a minimal ELF binary for amd64 Linux with no symbols, no
dwarfs, no dynamic linking, no stack, no data/rodata, no BSS, and no BS! To
build:

```
make
```

The `Makefile` uses `zig cc` by default so if that isn't your cup of tea you can
swap in your own compiler:

```
make CC=gcc
make CC=clang
```

The encoded instruction payload is built under `build/min_prog.text` and can be
viewed via:

```
make disass_text
```

It just exits with 20 as the status code. The instruction payload is 16 bytes
and the entire ELF binary is 136 bytes.

I wrote this as an experiment to learn more about the ELF format and to discover
what the least amount of data I need to provide Linux is until it starts
executing my instructions. It seems to be just the ELF header and a single LOAD
program header (segment).
