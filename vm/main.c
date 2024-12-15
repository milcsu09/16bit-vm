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
  VM vm = { 0 };
  vm_create (&vm, 256 * 256);

  byte program[] = {
    VM_OPERATION_MOV_R_I, VM_REGISTER_R1, LITERAL (255),
    VM_OPERATION_CALL_I, LITERAL (0x3000),

    VM_OPERATION_HALT,
  };

  /*
    [0x3000]
    def square(r1) -> ac:
      return r1 * r1
  */
  byte f_square[] = {
    VM_OPERATION_MUL_R, VM_REGISTER_AC, VM_REGISTER_R1, VM_REGISTER_R1,
    VM_OPERATION_RET,
  };

  memcpy (&vm.memory[0x0000], program, sizeof program);
  memcpy (&vm.memory[0x3000], f_square, sizeof f_square);

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
      if (waitpid(pid, &status, 0) > 0)
        {
          if (WIFEXITED(status))
            {
              int exit_code = WEXITSTATUS(status);
              fprintf(stderr, "Exited with code %i (%s)\n", exit_code,
                      vm_error_name (exit_code));
            }
          else if (WIFSIGNALED(status))
            printf("ERROR: terminated by signal %d\n", WTERMSIG(status));
        }
    }

  vm_destroy (&vm);

  return 0;
}

