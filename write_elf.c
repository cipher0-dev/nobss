#define _DEFAULT_SOURCE
#include <elf.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

void exit_usage(char *cmd) {
  fprintf(stderr, "usage: %s out_path\n", cmd);
  exit(EXIT_FAILURE);
}

void exit_perror(char *s) {
  perror(s);
  exit(EXIT_FAILURE);
}

char *get_arg(int argc, char *argv[]) {
  if (argc != 2)
    exit_usage(argv[0]);
  return argv[1];
}

// NOTE: This value depends on the size of the encoded instructions in the
//       program. Use `stat build/min_proc.text` to get this value.
#define TEXT_SIZE 16

#define ELF_SIZE (TEXT_OFFSET + TEXT_SIZE)
#define TEXT_OFFSET                                                            \
  (sizeof(Elf64_Ehdr) - UNUSED_EHDR_BYTES + sizeof(Elf64_Phdr) -               \
   UNUSED_PHDR_BYTES)
#define ENTRY_POINT (TEXT_OFFSET + VIRTUAL_OFFSET)
#define VIRTUAL_OFFSET 0x1001000
#define UNUSED_EHDR_BYTES (3 * sizeof(Elf64_Half))
#define UNUSED_PHDR_BYTES sizeof(Elf64_Xword)

int main(int argc, char *argv[]) {
  // setup outfile for writing

  auto out_path = get_arg(argc, argv);

  auto out_fd = open(out_path, O_RDWR | O_CREAT | O_TRUNC, 0755);
  if (out_fd == -1)
    exit_perror("open");

  if (ftruncate(out_fd, ELF_SIZE) == -1)
    perror("ftrucnate");

  char *elf_raw =
      mmap(NULL, ELF_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, out_fd, 0);
  auto elf = (Elf64_Ehdr *)elf_raw;
  if (elf == (void *)-1)
    exit_perror("mmap");

  if (close(out_fd) == -1)
    exit_perror("close");

  // write elf header

  printf("writing elf header into 0-%ld\n", sizeof(*elf) - UNUSED_EHDR_BYTES);

  elf->e_ident[EI_MAG0] = ELFMAG0;
  elf->e_ident[EI_MAG1] = ELFMAG1;
  elf->e_ident[EI_MAG2] = ELFMAG2;
  elf->e_ident[EI_MAG3] = ELFMAG3;

  elf->e_ident[EI_CLASS] = ELFCLASS64;
  elf->e_ident[EI_DATA] = ELFDATA2LSB;
  elf->e_ident[EI_VERSION] = EV_CURRENT;

  elf->e_type = ET_EXEC;
  elf->e_machine = EM_X86_64;
  elf->e_version = EV_CURRENT;
  elf->e_entry = ENTRY_POINT;

  // NOTE: The last three fields related to sections are not needed for runtime
  //       so I'm reusing those bytes for phdr data.
  elf->e_phoff = sizeof(*elf) - UNUSED_EHDR_BYTES;
  elf->e_ehsize = sizeof(*elf);
  elf->e_phentsize = sizeof(Elf64_Phdr);
  elf->e_phnum = 1;

  // write program header

  auto phdr = (Elf64_Phdr *)(elf_raw + elf->e_phoff);

  printf("writing program header into %ld-%ld\n", elf->e_phoff, TEXT_OFFSET);

  phdr->p_type = PT_LOAD;
  phdr->p_flags = PF_R | PF_X;
  // NOTE: The last phdr field for alignment isn't critical so I'm reusing those
  //       bytes for instruction data.
  phdr->p_offset = TEXT_OFFSET;
  phdr->p_vaddr = ENTRY_POINT;
  phdr->p_paddr = phdr->p_vaddr;
  phdr->p_filesz = TEXT_SIZE;
  phdr->p_memsz = phdr->p_filesz;

  // write instructions

  auto text_fd = open("./build/min_prog.text", O_RDONLY);
  if (text_fd == -1)
    exit_perror("open");

  char *text = mmap(NULL, TEXT_SIZE, PROT_READ, MAP_PRIVATE, text_fd, 0);
  if (text == (void *)-1)
    exit_perror("mmap");

  if (close(text_fd) == -1)
    exit_perror("close");

  printf("writing instructions into %ld-%ld\n", TEXT_OFFSET,
         TEXT_OFFSET + TEXT_SIZE);

  memcpy(elf_raw + TEXT_OFFSET, text, TEXT_SIZE);

  return EXIT_SUCCESS;
}
