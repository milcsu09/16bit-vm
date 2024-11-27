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
  load_file (vm, argv[1]);

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

