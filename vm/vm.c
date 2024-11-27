#include "vm.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

_Noreturn static void
vm_error (const char *fmt, ...)
{
  fprintf (stderr, "ERROR: ");

  va_list va;
  va_start (va, fmt);
  vfprintf (stderr, fmt, va);
  va_end (va);

  fprintf (stderr, "\n");
  exit (EXIT_FAILURE);
}

const char *
vm_operation_name (VM_Operation operation)
{
  return (operation < VM_ARRAY_SIZE (VM_OPERATION_NAME))
             ? VM_OPERATION_NAME[operation]
             : VM_UNKNOWN;
}

void
vm_create (VM *vm, word nmemb)
{
  vm->registers[VM_REGISTER_SP] = nmemb;
  vm->registers[VM_REGISTER_BP] = vm->registers[VM_REGISTER_SP];

  vm->ip = &vm->registers[VM_REGISTER_IP];
  vm->sp = &vm->registers[VM_REGISTER_SP];
  vm->bp = &vm->registers[VM_REGISTER_BP];

  vm->memory = calloc (nmemb + 1, sizeof (byte));
  vm->nmemb = nmemb;
  vm->halt = false;
}

void
vm_destroy (VM *vm)
{
  free (vm->memory);
  vm->memory = NULL;
  vm->nmemb = 0;
}

byte
vm_load_byte (VM *vm, word address)
{
  return vm->memory[address];
}

word
vm_load_word (VM *vm, word address)
{
  const byte H = vm_load_byte (vm, address + 0);
  const byte L = vm_load_byte (vm, address + 1);
  return VM_WORD_PACK (H, L);
}

word *
vm_load_register (VM *vm, word address)
{
  return &vm->registers[vm_load_byte (vm, address)];
}

byte
vm_next_byte (VM *vm)
{
  return vm_load_byte (vm, (*vm->ip)++);
}

word
vm_next_word (VM *vm)
{
  const byte H = vm_next_byte (vm);
  const byte L = vm_next_byte (vm);
  return VM_WORD_PACK (H, L);
}

word *
vm_next_register (VM *vm)
{
  return vm_load_register (vm, (*vm->ip)++);
}

void
vm_store_byte (VM *vm, word address, byte value)
{
  vm->memory[address] = value;
}

void
vm_store_word (VM *vm, word address, word value)
{
  const byte H = VM_WORD_H (value);
  const byte L = VM_WORD_L (value);
  vm_store_byte (vm, address + 0, H);
  vm_store_byte (vm, address + 1, L);
}

void
vm_push_byte (VM *vm, byte value)
{
  vm_store_byte (vm, *vm->sp, value);
  *vm->sp -= sizeof (byte);
}

void
vm_push_word (VM *vm, word value)
{
  const byte H = VM_WORD_H (value);
  const byte L = VM_WORD_L (value);
  vm_push_byte (vm, L);
  vm_push_byte (vm, H);
}

byte
vm_pop_byte (VM *vm)
{
  *vm->sp += sizeof (byte);
  return vm_load_byte (vm, *vm->sp);
}

word
vm_pop_word (VM *vm)
{
  const byte H = vm_pop_byte (vm);
  const byte L = vm_pop_byte (vm);
  return VM_WORD_PACK (H, L);
}

void
vm_compare (VM *vm, word left, word right)
{
  vm->flags.z = (left == right);
  vm->flags.c = (left < right);
}

void
vm_jump (VM *vm, word address, byte condition)
{
  if (condition)
    *vm->ip = address;
}

void
vm_execute (VM *vm, VM_Operation operation)
{
  switch (operation)
    {
    case VM_OPERATION_NOP:
      break;
    case VM_OPERATION_MOV_R_I:
      {
        word *dest = vm_next_register (vm);
        *dest = vm_next_word (vm);
      }
      break;
    case VM_OPERATION_MOV_R_R:
      {
        word *dest = vm_next_register (vm);
        *dest = *vm_next_register (vm);
      }
      break;
    case VM_OPERATION_MOV_R_IM:
      {
        word *dest = vm_next_register (vm);
        word address = vm_next_word (vm);
        *dest = vm_load_word (vm, address);
      }
      break;
    case VM_OPERATION_MOV_R_RM:
      {
        word *dest = vm_next_register (vm);
        word *address = vm_next_register (vm);
        *dest = vm_load_word (vm, *address);
      }
      break;
    case VM_OPERATION_MOV_IM_I:
      {
        word dest = vm_next_word (vm);
        word value = vm_next_word (vm);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_IM_R:
      {
        word dest = vm_next_word (vm);
        word *value = vm_next_register (vm);
        vm_store_word (vm, dest, *value);
      }
      break;
    case VM_OPERATION_MOV_IM_IM:
      {
        word dest = vm_next_word (vm);
        word address = vm_next_word (vm);
        word value = vm_load_word (vm, address);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_IM_RM:
      {
        word dest = vm_next_word (vm);
        word *address = vm_next_register (vm);
        word value = vm_load_word (vm, *address);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_RM_I:
      {
        word *dest = vm_next_register (vm);
        word value = vm_next_word (vm);
        vm_store_word (vm, *dest, value);
      }
      break;
    case VM_OPERATION_MOV_RM_R:
      {
        word *dest= vm_next_register (vm);
        word *value = vm_next_register (vm);
        vm_store_word (vm, *dest, *value);
      }
      break;
    case VM_OPERATION_MOV_RM_IM:
      {
        word *dest = vm_next_register (vm);
        word address = vm_next_word (vm);
        word value = vm_load_word (vm, address);
        vm_store_word (vm, *dest, value);
      }
      break;
    case VM_OPERATION_MOV_RM_RM:
      {
        word *dest = vm_next_register (vm);
        word *address = vm_next_register (vm);
        word value = vm_load_word (vm, *address);
        vm_store_word (vm, *dest, value);
      }
      break;
    case VM_OPERATION_PUSH_I:
      {
        word value = vm_next_word (vm);
        vm_push_word (vm, value);
      }
      break;
    case VM_OPERATION_PUSH_R:
      {
        word *value = vm_next_register (vm);
        vm_push_word (vm, *value);
      }
      break;
    case VM_OPERATION_POP:
      {
        word *dest = vm_next_register (vm);
        *dest = vm_pop_word (vm);
      }
      break;
    case VM_OPERATION_ADD_I:
      {
        word *dest = vm_next_register (vm);
        word *src1 = vm_next_register (vm);
        word src2 = vm_next_word (vm);
        *dest = *src1 + src2;
      }
      break;
    case VM_OPERATION_ADD_R:
      {
        word *dest = vm_next_register (vm);
        word *src1 = vm_next_register (vm);
        word *src2 = vm_next_register (vm);
        *dest = *src1 + *src2;
      }
      break;
    case VM_OPERATION_SUB_I:
      {
        word *dest = vm_next_register (vm);
        word *src1 = vm_next_register (vm);
        word src2 = vm_next_word (vm);
        *dest = *src1 - src2;
      }
      break;
    case VM_OPERATION_SUB_R:
      {
        word *dest = vm_next_register (vm);
        word *src1 = vm_next_register (vm);
        word *src2 = vm_next_register (vm);
        *dest = *src1 - *src2;
      }
      break;
    case VM_OPERATION_MUL_I:
      {
        word *dest = vm_next_register (vm);
        word *src1 = vm_next_register (vm);
        word src2 = vm_next_word (vm);
        *dest = *src1 * src2;
      }
      break;
    case VM_OPERATION_MUL_R:
      {
        word *dest = vm_next_register (vm);
        word *src1 = vm_next_register (vm);
        word *src2 = vm_next_register (vm);
        *dest = *src1 * *src2;
      }
      break;
    case VM_OPERATION_DIV_I:
      {
        word *dest = vm_next_register (vm);
        word *src1 = vm_next_register (vm);
        word src2 = vm_next_word (vm);
        *dest = *src1 / src2;
      }
      break;
    case VM_OPERATION_DIV_R:
      {
        word *dest = vm_next_register (vm);
        word *src1 = vm_next_register (vm);
        word *src2 = vm_next_register (vm);
        *dest = *src1 / *src2;
      }
      break;
    case VM_OPERATION_CMP_R_I:
      {
        word *left = vm_next_register (vm);
        word right = vm_next_word (vm);
        vm_compare (vm, *left, right);
      }
      break;
    case VM_OPERATION_CMP_R_R:
      {
        word *left = vm_next_register (vm);
        word *right = vm_next_register (vm);
        vm_compare (vm, *left, *right);
      }
      break;
    case VM_OPERATION_JMP_I:
      {
        word address = vm_next_word (vm);
        *vm->ip = address;
      }
      break;
    case VM_OPERATION_JMP_R:
      {
        word *address = vm_next_register (vm);
        *vm->ip = *address;
      }
      break;
    case VM_OPERATION_JEQ_I:
      {
        word address = vm_next_word (vm);
        vm_jump (vm, address, vm->flags.z == 1);
      }
      break;
    case VM_OPERATION_JEQ_R:
      {
        word *address = vm_next_register (vm);
        vm_jump (vm, *address, vm->flags.z == 1);
      }
      break;
    case VM_OPERATION_JNE_I:
      {
        word address = vm_next_word (vm);
        vm_jump (vm, address, vm->flags.z == 0);
      }
      break;
    case VM_OPERATION_JNE_R:
      {
        word *address = vm_next_register (vm);
        vm_jump (vm, *address, vm->flags.z == 0);
      }
      break;
    case VM_OPERATION_JLT_I:
      {
        word address = vm_next_word (vm);
        vm_jump (vm, address, vm->flags.c == 1);
      }
      break;
    case VM_OPERATION_JLT_R:
      {
        word *address = vm_next_register (vm);
        vm_jump (vm, *address, vm->flags.c == 1);
      }
      break;
    case VM_OPERATION_JGT_I:
      {
        word address = vm_next_word (vm);
        vm_jump (vm, address, vm->flags.z == 0 && vm->flags.c == 0);
      }
      break;
    case VM_OPERATION_JGT_R:
      {
        word *address = vm_next_register (vm);
        vm_jump (vm, *address, vm->flags.z == 0 && vm->flags.c == 0);
      }
      break;
    case VM_OPERATION_JLE_I:
      {
        word address = vm_next_word (vm);
        vm_jump (vm, address, vm->flags.c == 1 || vm->flags.z == 1);
      }
      break;
    case VM_OPERATION_JLE_R:
      {
        word *address = vm_next_register (vm);
        vm_jump (vm, *address, vm->flags.c == 1 || vm->flags.z == 1);
      }
      break;
    case VM_OPERATION_JGE_I:
      {
        word address = vm_next_word (vm);
        vm_jump (vm, address, vm->flags.c == 0);
      }
      break;
    case VM_OPERATION_JGE_R:
      {
        word *address = vm_next_register (vm);
        vm_jump (vm, *address, vm->flags.c == 0);
      }
      break;
    case VM_OPERATION_CALL_I:
      {
        word address = vm_next_word (vm);
        vm_push_word (vm, *vm->ip);
        *vm->ip = address;
      }
      break;
    case VM_OPERATION_CALL_R:
      {
        word *address = vm_next_register (vm);
        vm_push_word (vm, *vm->ip);
        *vm->ip = *address;
      }
      break;
    case VM_OPERATION_RET:
      *vm->ip = vm_pop_word (vm);
      break;
    case VM_OPERATION_HALT:
      vm->halt = true;
      break;
    default:
      vm_error ("unknown operation " VM_FMT_BYTE, (byte)operation);
    }
}

void
vm_step (VM *vm)
{
  vm_execute (vm, vm_next_byte (vm));
}

void
vm_view_registers (VM *vm)
{
  for (size_t i = 0; i < VM_REGISTER_COUNT; ++i)
    {
      const word value = vm->registers[i];
      printf ("%s: ", VM_REGISTER_NAME[i]);
      printf (value ? VM_FMT_WORD " (%d)\n" : "....\n", value, value);
    }
}

void
vm_view_memory (VM *vm, word address, word a, word b, bool decode)
{
  const word dec = (address >= a) ? a : address;
  const word inc = (address <= vm->nmemb - b) ? b : vm->nmemb - address;

  printf (VM_FMT_WORD ": ", address);

  for (size_t i = 0; i < a - dec; ++i)
    printf (".. ");

  for (size_t i = address - dec; i <= address + inc; ++i)
    printf (VM_FMT_BYTE " ", vm->memory[i]);

  for (size_t i = 0; i < b - inc; ++i)
    printf (".. ");

  printf ("\n%*s^", 6 + b * 3, "");

  if (decode)
    printf ("~ %s", vm_operation_name (vm->memory[address]));

  printf ("\n");
}

void
vm_view_flags (VM *vm)
{
  printf ("z=%d n=%d c=%d v=%d\n", vm->flags.z, vm->flags.n, vm->flags.c,
          vm->flags.v);
}

