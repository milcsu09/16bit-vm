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

  vm_view_registers (vm);
  printf ("\n");

  printf ("width=%i\n", (vm->width == VM_WIDTH_8) ? 8 : 16);
  vm_view_flags (vm);
  printf ("\n");

  vm_view_memory (vm, *vm->ip, 8, 8, true);
  vm_view_memory (vm, *vm->sp, 8, 8, false);

  // Sandbox
  vm_view_memory (vm, 0x3000, 8, 8, false);
}

int
main (void)
{
  VM vm = { 0 };
  vm_create (&vm, 0x10000);

  char *message = "Hello, world!";

  byte program[] = {
    VM_OPERATION_WIDTH_8,
    VM_OPERATION_MOV_R_IM, VM_REGISTER_R1, LITERAL (0x3000),
    VM_OPERATION_MOV_R_IM, VM_REGISTER_R2, LITERAL (0x3001),
    VM_OPERATION_MOV_R_IM, VM_REGISTER_R3, LITERAL (0x3002),
    VM_OPERATION_MOV_R_IM, VM_REGISTER_R4, LITERAL (0x3003),
    VM_OPERATION_MOV_R_IM, VM_REGISTER_R5, LITERAL (0x3004),
    VM_OPERATION_MOV_R_IM, VM_REGISTER_R6, LITERAL (0x3005),

    VM_OPERATION_HALT,
  };

  memcpy (&vm.memory[0x0000], program, sizeof program);
  memcpy (&vm.memory[0x3000], message, strlen (message));

  pid_t pid = fork ();
  if (pid == 0)
    while (1)
      {
        view_debug (&vm);
        if (getc (stdin) != '\n')
          continue;
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

