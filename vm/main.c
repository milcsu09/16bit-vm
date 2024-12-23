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
  vm_view_memory (vm, *vm->sp, 12, 4, false);
  vm_view_memory (vm, 0x7000, 12, 4, false);

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
  vm_create (&vm, 0x10000);
  load_file (&vm, argv[1]);

  pid_t pid = fork ();
  if (pid == 0)
    while (1)
      {
        if (vm.memory[*vm.ip] == VM_OPERATION_HALT)
          view_io (&vm);
        // view_debug (&vm);
        // if (getc (stdin) != '\n')
        //   continue;
        vm_step (&vm);
        // usleep (5000);
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

/*#include "vm.h"
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
  vm_view_memory (vm, 0x7000, 12, 4, false);

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
    // VM_OPERATION_ADD_I | 0x80, VM_REGISTER_R2, VM_REGISTER_R2, 1,
    // VM_OPERATION_ADD_I | 0x80, VM_REGISTER_R1, VM_REGISTER_R1, 1,

    // VM_OPERATION_CMP_I,        VM_REGISTER_R1, LITERAL (0xffff),
    // VM_OPERATION_JNE_I,        LITERAL (0x0004),

    // VM_OPERATION_CMP_I,        VM_REGISTER_R2, LITERAL (0xff),
    // VM_OPERATION_JNE_I,        LITERAL (0x0000),

0x85, 0x00, 0x70, 0x48, 0x85, 0x01, 0x70, 0x65, 0x85, 0x02, 0x70, 0x6c, 0x85, 0x03, 0x70, 0x6c, 0x85, 0x04, 0x70, 0x6f, 0x85, 0x05, 0x70, 0x21, 0x2b,
  };

  memcpy (&vm.memory[0x0000], program, sizeof program);

  pid_t pid = fork ();
  if (pid == 0)
    while (1)
      {
#if 0
        if (vm.memory[*vm.ip] == VM_OPERATION_HALT)
          view_debug (&vm);
#else
        view_debug (&vm);
        if (getc (stdin) != '\n')
          continue;
#endif
        vm_step (&vm);
        // usleep (5000);
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
}*/

