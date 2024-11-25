#include "vm.h"
#include <stdlib.h>

int
main (void)
{
  VM *vm = malloc (sizeof (VM));
  vm_create (vm, 0xffff);
  vm_destroy (vm);

  return 0;
}

