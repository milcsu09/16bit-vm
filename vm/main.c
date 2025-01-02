#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

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
view_io (VM *vm)
{
  for (word i = 0x7000; vm->memory[i] != 0; ++i)
    printf ("%c", vm->memory[i]);
  printf ("\n");
}

static inline void
view_debug (VM *vm)
{
  printf ("\033[2J\033[H");
  printf ("\n");

  for (size_t i = 0; i < VM_REGISTER_COUNT; ++i)
    vm_view_register (vm, i);
  printf ("\n");

  vm_view_memory (vm, *vm->ip, 12, 4, true);
  view_io (vm);
}

int
main (int argc, char *argv[argc])
{
  if (argc != 2)
    {
      fprintf (stderr, "error: Provide ROM file\n");
      exit (1);
    }

  VM vm = { 0 };
  vm_create (&vm);
  load_file (&vm, argv[1]);

  pid_t pid = fork ();
  if (pid == 0)
    while (1)
      {
        // if (vm.memory[*vm.ip] == VM_OPERATION_HALT)
        //   view_io (&vm);

        // view_debug (&vm);
        // if (getc (stdin) != '\n')
        //   continue;
    
        // if (vm.memory[*vm.ip] == VM_OPERATION_HALT)
        //  view_debug (&vm);

        vm_step (&vm);
      }
  else
    {
      int status;
      if (waitpid (pid, &status, 0) > 0)
        {
          if (WIFEXITED (status))
            {
              int exit_code = WEXITSTATUS (status);
              if (exit_code != 0)
                fprintf (stderr, "error: process exited with code %i (%s)\n",
                         exit_code,
                         vm_error_name (exit_code));
              else
                printf ("OK.\n");
            }
          else if (WIFSIGNALED (status))
            printf ("error: process terminated by signal %d\n",
                    WTERMSIG (status));
        }
    }

  vm_destroy (&vm);

  return 0;
}

