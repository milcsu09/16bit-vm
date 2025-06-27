#include "../vm/vm.h"

#include <stdio.h>

void
writer_store_byte (VM *vm, VM_Device *device, word address, byte value)
{
  (void)vm, (void)device, (void)address;
  putc (value, stdout);
}

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

  VM_Device writer = {0};

  writer.read_byte = vm_default_read_byte;
  writer.read_word = vm_default_read_word;
  writer.store_byte = writer_store_byte;
  writer.store_word = vm_default_store_word;
  writer.state = NULL;

  vm_map_device (&vm, &writer, 0x3000, 0x3100);

  if (!vm_load_file (&vm, argv[1]))
    return 1;

  while (!vm.halt)
    {
      vm_step (&vm);
    }

  vm_destroy (&vm);
}

