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
  for (word i = 0x7000; i < 0x70ff; ++i)
    if (vm->memory[i])
      printf ("%c", vm->memory[i]);
  printf ("\n");
}

int
main (void)
{
  VM vm = { 0 };
  vm_create (&vm, 0x10000);

  byte alphabet[] = {
    VM_OPERATION_MOV8_R_I, VM_REGISTER_R1, 'a',
    VM_OPERATION_MOV_R_I, VM_REGISTER_R2, LITERAL (0x7000),

// .loop:
    VM_OPERATION_MOV_R_R, VM_REGISTER_R3, VM_REGISTER_R1,

    VM_OPERATION_DIV_I, VM_REGISTER_R8, VM_REGISTER_R3, LITERAL (2),

    VM_OPERATION_CMP8_R_I, VM_REGISTER_AC, 1,
    VM_OPERATION_JEQ_I, LITERAL (26),

    VM_OPERATION_SUB_I, VM_REGISTER_R3, VM_REGISTER_R3, LITERAL (32),

// .skip
    VM_OPERATION_MOV8_RM_R, VM_REGISTER_R2, VM_REGISTER_R3,

    VM_OPERATION_ADD_I, VM_REGISTER_R1, VM_REGISTER_R1, LITERAL (1),
    VM_OPERATION_ADD_I, VM_REGISTER_R2, VM_REGISTER_R2, LITERAL (1),

    VM_OPERATION_CMP8_R_I, VM_REGISTER_R1, 'z',
    VM_OPERATION_JLE_I, LITERAL (7),

    VM_OPERATION_HALT,
  };

  byte collatz[] = {
    VM_OPERATION_MOV_R_I, VM_REGISTER_R1, LITERAL (15),

    VM_OPERATION_ADD_I, VM_REGISTER_R2, VM_REGISTER_R2, LITERAL (1),
    VM_OPERATION_DIV_I, VM_REGISTER_R8, VM_REGISTER_R1, LITERAL (2),
    VM_OPERATION_CMP8_R_I, VM_REGISTER_AC, 0,
    VM_OPERATION_JEQ_I, LITERAL (32),

    VM_OPERATION_MUL_I, VM_REGISTER_R1, VM_REGISTER_R1, LITERAL (3),
    VM_OPERATION_ADD_I, VM_REGISTER_R1, VM_REGISTER_R1, LITERAL (1),
    VM_OPERATION_JMP_I, LITERAL (4),

    VM_OPERATION_DIV_I, VM_REGISTER_R1, VM_REGISTER_R1, LITERAL (2),
    VM_OPERATION_JMP_I, LITERAL (4),

    VM_OPERATION_HALT
  };

  byte main[] = {
    VM_OPERATION_MOV_R_I, VM_REGISTER_R1, LITERAL (0xffff),
    VM_OPERATION_MOV_R_I, VM_REGISTER_R2, LITERAL (0x7004),
    VM_OPERATION_CALL_I, LITERAL (0x2000),

    VM_OPERATION_MOV8_IM_I, LITERAL (0x7005), ' ',

    VM_OPERATION_MOV_R_I, VM_REGISTER_R1, LITERAL (1234),
    VM_OPERATION_MOV_R_I, VM_REGISTER_R2, LITERAL (0x7009),
    VM_OPERATION_CALL_I, LITERAL (0x2000),

    VM_OPERATION_HALT,
  };

  // putd (r1 n, r2 address) -> void
  byte putd[] = {
    VM_OPERATION_DIV_I, VM_REGISTER_R1, VM_REGISTER_R1, LITERAL (10),
    VM_OPERATION_ADD_I, VM_REGISTER_AC, VM_REGISTER_AC, '0', 0x00,

    VM_OPERATION_MOV8_RM_R, VM_REGISTER_R2, VM_REGISTER_AC,
    VM_OPERATION_SUB_I, VM_REGISTER_R2, VM_REGISTER_R2, LITERAL (1),

    VM_OPERATION_CMP_R_I, VM_REGISTER_R1, LITERAL (0),
    VM_OPERATION_JNE_I, LITERAL (0x2000),

    VM_OPERATION_RET,
  };

  memcpy (&vm.memory[0x0000], main, sizeof main);
  memcpy (&vm.memory[0x2000], putd, sizeof putd);
  // memcpy (&vm.memory[0x0000], alphabet, sizeof alphabet);

  pid_t pid = fork ();
  if (pid == 0)
    while (1)
      {
        // if (vm.memory[*vm.ip] == VM_OPERATION_HALT)
        //   view_debug (&vm);

        view_debug (&vm);
        // if (getc (stdin) != '\n')
        //   continue;

        vm_step (&vm);
        usleep (50000);
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

