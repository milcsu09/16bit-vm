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
  (void)argc, (void)argv;

  VM *vm = calloc (1, sizeof (VM));
  vm_create (vm, 256 * 256);

#if 0
  if (argc != 2)
    {
      fprintf (stderr, "ERROR: Provide input binary file\n");
      exit (1);
    }

  load_file (vm, argv[1]);
#else
  byte program[] = {
    VM_OPERATION_MOV_R_I, VM_REGISTER_R1, 0xAB, 0xCD, 
    VM_OPERATION_MOV_R_I, VM_REGISTER_R2, 0x12, 0x34, 

    VM_OPERATION_PUSH_R, VM_REGISTER_R1,
    VM_OPERATION_PUSH_R, VM_REGISTER_R2,

    VM_OPERATION_POP, VM_REGISTER_R1,
    VM_OPERATION_POP, VM_REGISTER_R2,

    VM_OPERATION_HALT,
  };

  memcpy (vm->memory, program, sizeof program);
#endif

  while (!vm->halt)
    vm_step (vm);

  if (vm_success (vm))
    vm_view_registers (vm);
  else
    fprintf (stderr, "ERROR: %s\n", vm->error);

  /*
  view_debug (vm);

  while (!vm->halt)
    {
      if (getc (stdin) != '\n')
        continue;

      vm_step (vm);
      view_debug (vm);
    }

  if (!vm_success (vm))
    fprintf (stderr, "ERROR: %s\n", vm->error);
  */

  vm_destroy (vm);
  free (vm);

  return 0;
}

