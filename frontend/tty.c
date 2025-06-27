#include "../vm/vm.h"

#include <stdio.h>

int
main (int argc, char **argv)
{
  if (argc <= 1)
    {
      fprintf (stderr, "USAGE: %s <ROM>\n", argv[0]);
      return 1;
    }

  VM vm = {0};

  vm_create (&vm);

  if (!vm_load_file (&vm, argv[1]))
    return 1;

  while (!vm.halt)
    {
      vm_step (&vm);
    }

  vm_destroy (&vm);
}

