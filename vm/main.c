
#include "vm.h"
#include <stdio.h>
#include <string.h>

int
main()
{
  VM vm = { 0 };
  vm_create (&vm);

  byte program[] = {
    VM_OPERATION_MOV_R_I, VM_REGISTER_R1, 32,
    VM_OPERATION_MOV_R_I, VM_REGISTER_R2, 4,

    VM_OPERATION_SHR_R, VM_REGISTER_R3, VM_REGISTER_R1, VM_REGISTER_R2,

    VM_OPERATION_HALT,
  };

  memcpy (vm.memory, program, sizeof program);

  while (!vm.halt)
    {
      printf ("\033[2J\033[H");
      printf ("\n");

      for (byte i = 0; i < VM_REGISTER_COUNT; ++i)
        vm_view_register (&vm, i);

      vm_view_memory (&vm, *vm.ip, 4, 12, true);

      if (getc (stdin) != '\n')
        continue;

      vm_step (&vm);
    }

  return 0;
}

