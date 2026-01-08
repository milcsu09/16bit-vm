#include "../vm/vm.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_REGIONS_SIZE 16

struct memory_region
{
  word address;
  word below;
  word above;
  int decode;
};


int
main (int argc, char **argv)
{
  if (argc <= 1)
    {
      fprintf (stderr, "USAGE: %s <ROM>\n", argv[0]);
      return 1;
    }

  struct memory_region regions[MAX_REGIONS_SIZE];
  size_t regions_size = 0;

  bool view_registers = 1;
  bool view_region_ip = 1;
  bool view_region_sp = 0;
  bool view_region_bp = 0;

  bool view_flags = 1;

  VM vm = {0};

  vm_create (&vm);

  if (!vm_load_file (&vm, argv[1]))
    return 1;

  char line[512];

  while (!vm.halt)
    {
      printf ("\033[2J\033[H");
      printf ("\n");

      if (view_registers)
        {
          for (int i = 0; i < VM_REGISTER_COUNT; ++i)
            vm_view_register (&vm, i);
          printf ("\n");
        }

      if (view_region_ip)
        {
          vm_view_memory (&vm, *vm.ip, 4, 12, 1);
          printf ("\n");
        }

      if (view_region_sp)
        {
          vm_view_memory (&vm, *vm.sp, 4, 12, 0);
          printf ("\n");
        }

      if (view_region_bp)
        {
          vm_view_memory (&vm, *vm.bp, 8, 8, 0);
          printf ("\n");
        }

      if (view_flags)
        {
          printf ("z c\n");
          printf ("%d %d\n", vm.flags.z, vm.flags.c);
          printf ("\n");
        }

      for (size_t i = 0; i < regions_size; ++i)
        {
          struct memory_region region = regions[i];
          vm_view_memory (&vm, region.address, region.below, region.above,
                          region.decode);
          printf ("\n");
        }

read_line:
      printf ("'");
      if (!fgets (line, sizeof line, stdin))
        {
          vm.halt = true;
          break;
        }

      line[strcspn (line, "\n")] = 0;

      char *command = strtok (line, " ");
      if (!command)
        goto read_line;

      char *arg1 = strtok (NULL, " ");
      char *arg2 = strtok (NULL, " ");
      char *arg3 = strtok (NULL, " ");
      char *arg4 = strtok (NULL, " ");

      if (strcmp (command, "r") == 0)
        view_registers = !view_registers;

      if (strcmp (command, "rs") == 0)
        {
          if (arg1 && arg2)
            {
              byte r = strtol (arg1, NULL, 0);
              word v = strtol (arg2, NULL, 0);
              vm.registers[r % VM_REGISTER_COUNT] = v;
            }
        }

      if (strcmp (command, "mvip") == 0)
        view_region_ip = !view_region_ip;

      if (strcmp (command, "mvsp") == 0)
        view_region_sp = !view_region_sp;

      if (strcmp (command, "mvbp") == 0)
        view_region_bp = !view_region_bp;

      if (strcmp (command, "mv") == 0)
        {
          if (arg1 && (!arg2 && !arg3 && !arg4))
            {
              int address = strtol (arg1, NULL, 0);

              size_t index = -1;
              for (size_t i = 0; i < regions_size; ++i)
                {
                  if (regions[i].address == address)
                    index = i;
                }

              if (index != (size_t)-1)
                {
                  regions_size--;
                  for (size_t i = index; i < regions_size; ++i)
                    regions[i] = regions[i + 1];
                }
            }

          if (regions_size < MAX_REGIONS_SIZE && arg1 && arg2 && arg3 && arg4)
            {
              struct memory_region region;

              region.address = strtol (arg1, NULL, 0);
              region.below = strtol (arg2, NULL, 0);
              region.above = strtol (arg3, NULL, 0);
              region.decode = strtol (arg4, NULL, 0);

              regions[regions_size++] = region;
            }
        }

      if (strcmp (command, "F") == 0)
        view_flags = !view_flags;

      if (strcmp (command, "puts") == 0)
        {
          if (arg1)
            {
              const char *s = (char *)&vm.memory[(word)strtol (arg1, NULL, 0)];
              printf ("%s\n", s);
              goto read_line;
            }
        }

      if (strcmp (command, "s") == 0)
        {
          if (arg1)
            {
              int n = strtol (arg1, NULL, 0);
              for (int i = 0; i < n && !vm.halt; ++i)
                vm_step (&vm);
            }
          else
            vm_step (&vm);
        }

      if (strcmp (command, "n") == 0)
        {
          word base = *vm.sp;
          vm_step (&vm);
          while (*vm.sp < base && !vm.halt)
            vm_step (&vm);
        }

      if (strcmp (command, "f") == 0)
        {
          word base = *vm.sp;
          while (*vm.sp <= base && !vm.halt)
            vm_step (&vm);
        }
    }

  vm_destroy (&vm);

  return 0;
}

