#include "vm.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define LITERAL(w) VM_WORD_H (w), VM_WORD_L (w)

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
main (void)
{
  VM *vm = calloc (1, sizeof (VM));
  vm_create (vm, 256 * 256);

  byte program[] = {
    VM_OPERATION_MOV_R_IM, VM_REGISTER_R1, LITERAL (0xffff),
    VM_OPERATION_HALT,
  };

  memcpy (vm->memory, program, sizeof program);

  pid_t pid = fork ();
  if (pid == 0)
    while (1)
      {
        view_debug (vm);
 
        if (getc (stdin) != '\n')
          continue;

        vm_step (vm);
      }
  else
    {
      int status;
      waitpid(pid, &status, 0);

      if (WIFEXITED(status))
        {
          int code = WEXITSTATUS(status);
          if (code != 0)
            fprintf (stderr, "ERROR: exited with code %i (%s)\n", code,
                     vm_error_name (code));
          else
            printf ("OK\n");
        }
      else
        fprintf (stderr, "ERROR: exited abnormally\n");
    }

  vm_destroy (vm);
  free (vm);

  return 0;
}

