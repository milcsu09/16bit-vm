#include "vm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static inline void
load_file (VM *vm, const char *path)
{
  FILE *file = fopen (path, "rb");

  fseek(file, 0, SEEK_END);
  long nmemb = ftell (file);
  rewind(file);

  byte *bytecode = malloc (nmemb);
  size_t nread = fread (bytecode, 1, nmemb, file);

  fclose (file);

  memcpy (vm->memory, bytecode, nread);
  free (bytecode);
}

static inline void
view_debug (VM *vm)
{
  printf ("\033[2J\033[H");
  printf ("\n");

  vm_view_registers (vm);
  printf ("\n");

  vm_view_flags (vm);
  printf ("\n");

  vm_view_memory (vm, *vm->ip, 8, 8, true);
  vm_view_memory (vm, *vm->sp, 8, 8, false);
}

int
main (int argc, char *argv[])
{
  if (argc != 2)
    {
      fprintf (stderr, "ERROR: Provide input binary file\n");
      exit (1);
    }

  VM *vm = malloc (sizeof (VM));
  vm_create (vm, 0xffff);

#define FROM_FILE 0

#if FROM_FILE
  load_file (vm, argv[1]);
#else
  byte program[] = {
    VM_OPERATION_MOV_R_I, VM_REGISTER_R1, 0x12, 0xAB,
    VM_OPERATION_MOV_R_I, VM_REGISTER_R2, 0x12, 0xAB,
    VM_OPERATION_CMP_R_R, VM_REGISTER_R1, VM_REGISTER_R2,

    VM_OPERATION_HALT,
  };

  memcpy (vm->memory, program, sizeof program);
#endif

  view_debug (vm);

  while (!vm->halt)
    {
      if (getc (stdin) != '\n')
        continue;

      vm_step (vm);
      view_debug (vm);
    }

  vm_destroy (vm);
  free (vm);

  return 0;
}

