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

#if 1
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
    VM_OPERATION_HALF,                                        // 0
    VM_OPERATION_MOV_R_I, VM_REGISTER_R1, 'a',                // 3
    VM_OPERATION_MOV_R_I, VM_REGISTER_R2, LITERAL (0x7000),   // 7

                                                              // .loop - 8
    VM_OPERATION_MOV_R_R, VM_REGISTER_R3, VM_REGISTER_R1,     // 10

    VM_OPERATION_HALF,                                        // 11
    VM_OPERATION_DIV_I, VM_REGISTER_R8, VM_REGISTER_R3, 2,    // 15

    VM_OPERATION_HALF,                                        // 16
    VM_OPERATION_CMP_I, VM_REGISTER_AC, 1,                    // 19

    VM_OPERATION_JEQ_I, LITERAL (0x001c),                     // 22

    VM_OPERATION_HALF,                                        // 23
    VM_OPERATION_SUB_I, VM_REGISTER_R3, VM_REGISTER_R3, 32,   // 27

                                                              // .skip - 28
    VM_OPERATION_HALF,                                        // 28
    VM_OPERATION_MOV_RM_R, VM_REGISTER_R2, VM_REGISTER_R3,    // 31

    VM_OPERATION_HALF,                                        // 32
    VM_OPERATION_ADD_I, VM_REGISTER_R1, VM_REGISTER_R1, 1,    // 36

    VM_OPERATION_HALF,                                        // 37
    VM_OPERATION_ADD_I, VM_REGISTER_R2, VM_REGISTER_R2, 1,    // 41

    VM_OPERATION_HALF,                                        // 42
    VM_OPERATION_CMP_I, VM_REGISTER_R1, 'z',                  // 45

    VM_OPERATION_JLE_I, LITERAL (0x0008),                     // 48


    VM_OPERATION_HALT,                                        // 49
  };

  memcpy (&vm.memory[0x0000], program, sizeof program);

  pid_t pid = fork ();
  if (pid == 0)
    while (1)
      {
        if (vm.memory[*vm.ip] == VM_OPERATION_HALT)
          view_debug (&vm);
        // view_debug (&vm);
        // if (getc (stdin) != '\n')
        //   continue;
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

