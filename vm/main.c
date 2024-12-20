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

  vm_view_flags (vm);
  printf ("\n");

  vm_view_memory (vm, *vm->ip, 8, 8, true);
  vm_view_memory (vm, *vm->sp, 8, 8, false);
  vm_view_memory (vm, 0x7000, 8, 8, false);

  printf ("\n");
  for (word i = 0x7000; vm->memory[i] != 0; ++i)
    printf ("%c", vm->memory[i]);
  printf ("\n");

  // Sandbox
  // vm_view_memory (vm, 0x3000, 8, 8, false);
}

int
main (void)
{
  VM vm = { 0 };
  vm_create (&vm, 0x10000);

  char *message = "Hello, world!";

  byte program[] = {
    VM_OPERATION_MOV16_R_I, VM_REGISTER_R1, LITERAL (0x3000),
    VM_OPERATION_ADD_R, VM_REGISTER_R1, VM_REGISTER_R1, VM_REGISTER_R2,

    VM_OPERATION_MOV8_R_RM, VM_REGISTER_R1, VM_REGISTER_R1,

    VM_OPERATION_MOV16_R_I, VM_REGISTER_R4, LITERAL (0x7000),
    VM_OPERATION_ADD_R, VM_REGISTER_R4, VM_REGISTER_R4, VM_REGISTER_R2,

    VM_OPERATION_MOV8_RM_R, VM_REGISTER_R4, VM_REGISTER_R1,

    VM_OPERATION_ADD_I, VM_REGISTER_R2, VM_REGISTER_R2, LITERAL (1),

    VM_OPERATION_CMP_R_I, VM_REGISTER_R1, LITERAL (0),
    VM_OPERATION_JNE_I, LITERAL (0),

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

