#include "vm.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define LITERAL(w) VM_WORD_L (w), VM_WORD_H (w)

static inline void
view_debug (VM *vm)
{
  printf ("\033[2J\033[H");
  printf ("\n");

  for (size_t i = 0; i < VM_REGISTER_COUNT; ++i)
    vm_view_register (vm, i);
  printf ("\n");

  vm_view_memory (vm, *vm->ip, 12, 4, true);
  vm_view_memory (vm, *vm->sp, 12, 4, false);

#if 0
  printf ("\n");
  for (word i = 0x7000; i < 0x70ff; ++i)
    if (vm->memory[i])
      printf ("%c", vm->memory[i]);
  printf ("\n");
#endif
}

int
main (void)
{
  VM vm = { 0 };
  vm_create (&vm, 0x10000);

  byte program[] = {
    VM_OPERATION_HALT,
  };

  memcpy (&vm.memory[0x0000], program, sizeof program);

  pid_t pid = fork ();
  if (pid == 0)
    while (1)
      {
        view_debug (&vm);
        if (getc (stdin) != '\n')
          continue;
        vm_step (&vm);
        // usleep (500);
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

