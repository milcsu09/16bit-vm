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
  /*printf ("\033[2J\033[H");
  printf ("\n");

  vm_view_registers (vm);
  printf ("\n");

  vm_view_flags (vm);
  printf ("\n");

  vm_view_memory (vm, *vm->ip, 8, 8, true);
  vm_view_memory (vm, *vm->sp, 8, 8, false);*/

  // byte *io = &vm->memory[0x3000];
  byte buffer[16 * 16] = { 0 };
  word buffer_sz = 0;

  for (word i = 0x3000; i <= 0x30ff; i += 2)
    {
      word value = vm_load_word (vm, i);
      byte E = VM_WORD_H (value);
      byte C = VM_WORD_L (value);

      buffer[buffer_sz++] = C;
    }

  printf ("%s", buffer);
}

int
main (int argc, char *argv[])
{
  VM *vm = calloc (1, sizeof (VM));
  vm_create (vm, 256 * 256);

#if 1
  if (argc != 2)
    {
      fprintf (stderr, "ERROR: Provide input binary file\n");
      exit (1);
    }

  load_file (vm, argv[1]);
#else
  byte program[] = {
    // VM_OPERATION_MOV_R_I, VM_REGISTER_R1, 0xAB, 0xCD, 
    // VM_OPERATION_MOV_R_I, VM_REGISTER_R2, 0x12, 0x34, 

    // VM_OPERATION_PUSH_R, VM_REGISTER_R1,
    // VM_OPERATION_PUSH_R, VM_REGISTER_R2,

    // VM_OPERATION_POP, VM_REGISTER_R1,
    // VM_OPERATION_POP, VM_REGISTER_R2,

    VM_OPERATION_MOV_IM_I, 0x30, 0x00, 0x00, 'H',
    VM_OPERATION_MOV_IM_I, 0x30, 0x02, 0x00, 'e',
    VM_OPERATION_MOV_IM_I, 0x30, 0x04, 0x00, 'l',
    VM_OPERATION_MOV_IM_I, 0x30, 0x06, 0x00, 'l',
    VM_OPERATION_MOV_IM_I, 0x30, 0x08, 0x00, 'o',
    VM_OPERATION_MOV_IM_I, 0x30, 0x0a, 0x00, '!',
    VM_OPERATION_MOV_IM_I, 0x30, 0x0c, 0x00, '\n',

    VM_OPERATION_MOV_R_IM, VM_REGISTER_R1, 0x30, 0x04,
    VM_OPERATION_ADD_I, VM_REGISTER_R1, VM_REGISTER_R1, 0x00, 1,
    VM_OPERATION_MOV_IM_R, 0x30, 0x04, VM_REGISTER_R1,

    VM_OPERATION_HALT,
  };

  memcpy (vm->memory, program, sizeof program);
#endif

  // view_debug (vm);

  while (!vm->halt)
    {
      // if (getc (stdin) != '\n')
      //   continue;

      vm_step (vm);
      // view_debug (vm);
    }

  view_debug (vm);
  vm_destroy (vm);
  free (vm);

  return 0;
}

