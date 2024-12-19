#include "vm.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const char *const VM_REGISTER_NAME[] = {
  "ip", "sp", "bp", "ac", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
};

static const char *const VM_OPERATION_NAME[] = {
  "none",
  "mov(r, i)",
  "mov(r, r)",
  "mov(r, im)",
  "mov(r, rm)",
  "mov(im, i)",
  "mov(im, r)",
  "mov(im, im)",
  "mov(im, rm)",
  "mov(rm, i)",
  "mov(rm, r)",
  "mov(rm, im)",
  "mov(rm, rm)",
  "push(i)",
  "push(r)",
  "pop",
  "add(i)",
  "add(r)",
  "sub(i)",
  "sub(r)",
  "mul(i)",
  "mul(r)",
  "div(i)",
  "div(r)",
  "cmp(r, i)",
  "cmp(r, r)",
  "jmp(i)",
  "jmp(r)",
  "jeq(i)",
  "jeq(r)",
  "jne(i)",
  "jne(r)",
  "jlt(i)",
  "jlt(r)",
  "jgt(i)",
  "jgt(r)",
  "jle(i)",
  "jle(r)",
  "jge(i)",
  "jge(r)",
  "call(i)",
  "call(r)",
  "ret",
  "halt",
};

static const char *const VM_ERROR_NAME[] = {
  "none",
  "illegal operation",
  "memory violation",
};

static_assert (VM_ARRAY_SIZE (VM_REGISTER_NAME) == VM_REGISTER_COUNT,
               "items not aligned in VM_REGISTER_NAME");

static_assert (VM_ARRAY_SIZE (VM_OPERATION_NAME) == VM_OPERATION_COUNT,
               "items not aligned in VM_OPERATION_NAME");

static_assert (VM_ARRAY_SIZE (VM_ERROR_NAME) == VM_ERROR_COUNT,
               "items not aligned in VM_ERROR_NAME");

static inline void
vm_assert (bool condition, VM_Error error)
{
  if (!condition)
    exit (error);
}

static inline char *
vm_module_name (size_t index, size_t n, const char *const xs[n])
{
  return (char *)(index < n ? xs[index] : "invalid");
}

char *
vm_operation_name (VM_Operation index)
{
  return vm_module_name (index, VM_OPERATION_COUNT, VM_OPERATION_NAME);
}

char *
vm_error_name (VM_Error index)
{
  return vm_module_name (index, VM_ERROR_COUNT, VM_ERROR_NAME);
}

void
vm_create (VM *vm, size_t nmemb)
{
  vm->registers[VM_REGISTER_SP] = nmemb - sizeof (word);
  vm->registers[VM_REGISTER_BP] = vm->registers[VM_REGISTER_SP];

  vm->ip = &vm->registers[VM_REGISTER_IP];
  vm->sp = &vm->registers[VM_REGISTER_SP];
  vm->bp = &vm->registers[VM_REGISTER_BP];

  vm->memory = calloc (nmemb, sizeof (byte));
  vm->nmemb = nmemb;
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
  vm_assert (address < vm->nmemb, VM_ERROR_MEMORY_VIOLATION);

  return vm->memory[address];
}

word
vm_load_word (VM *vm, word address)
{
  const byte H = vm_load_byte (vm, address + 0);
  const byte L = vm_load_byte (vm, address + 1);
  return VM_WORD_PACK (H, L);
}

word
vm_load_register_value (VM *vm, word address)
{
  byte index = vm_load_byte (vm, address);

  vm_assert (index < VM_REGISTER_COUNT, VM_ERROR_ILLEGAL_OPERATION);

  return vm->registers[index];
}

word *
vm_load_register_address (VM *vm, word address)
{
  byte index = vm_load_byte (vm, address);

  vm_assert (index < VM_REGISTER_COUNT, VM_ERROR_ILLEGAL_OPERATION);

  return &vm->registers[index];
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

word
vm_next_register_value (VM *vm)
{
  return vm_load_register_value (vm, (*vm->ip)++);
}

word *
vm_next_register_address (VM *vm)
{
  return vm_load_register_address (vm, (*vm->ip)++);
}

void
vm_store_byte (VM *vm, word address, byte value)
{
  vm_assert (address < vm->nmemb, VM_ERROR_MEMORY_VIOLATION);

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
  vm_assert (*vm->sp >= sizeof (byte), VM_ERROR_MEMORY_VIOLATION);

  vm_store_byte (vm, *vm->sp, value);
  *vm->sp -= sizeof (byte);
}

void
vm_push_word (VM *vm, word value)
{
  vm_assert (*vm->sp >= sizeof (word), VM_ERROR_MEMORY_VIOLATION);

  vm_store_word (vm, *vm->sp, value);
  *vm->sp -= sizeof (word);
}

byte
vm_pop_byte (VM *vm)
{
  vm_assert (*vm->sp < vm->nmemb - sizeof (byte), VM_ERROR_MEMORY_VIOLATION);

  *vm->sp += sizeof (byte);
  return vm_load_byte (vm, *vm->sp);
}

word
vm_pop_word (VM *vm)
{
  vm_assert (*vm->sp < vm->nmemb - sizeof (word), VM_ERROR_MEMORY_VIOLATION);

  *vm->sp += sizeof (word);
  return vm_load_word (vm, *vm->sp);
}

void
vm_compare (VM *vm, word left, word right)
{
  vm->flags.z = (left == right);
  vm->flags.c = (left < right);
}

void
vm_jump (VM *vm, word address, bool condition)
{
  if (condition)
    *vm->ip = address;
}

void
vm_execute (VM *vm, VM_Operation operation)
{
  switch (operation)
    {
    case VM_OPERATION_NONE:
      break;
    case VM_OPERATION_MOV_R_I:
      {
        word *dest = vm_next_register_address (vm);
        *dest = vm_next_word (vm);
      }
      break;
    case VM_OPERATION_MOV_R_R:
      {
        word *dest = vm_next_register_address (vm);
        *dest = vm_next_register_value (vm);
      }
      break;
    case VM_OPERATION_MOV_R_IM:
      {
        word *dest = vm_next_register_address (vm);
        word address = vm_next_word (vm);
        *dest = vm_load_word (vm, address);
      }
      break;
    case VM_OPERATION_MOV_R_RM:
      {
        word *dest = vm_next_register_address (vm);
        word address = vm_next_register_value (vm);
        *dest = vm_load_word (vm, address);
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
        word value = vm_next_register_value (vm);
        vm_store_word (vm, dest, value);
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
        word address = vm_next_register_value (vm);
        word value = vm_load_word (vm, address);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_RM_I:
      {
        word dest = vm_next_register_value (vm);
        word value = vm_next_word (vm);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_RM_R:
      {
        word dest= vm_next_register_value (vm);
        word value = vm_next_register_value (vm);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_RM_IM:
      {
        word dest = vm_next_register_value (vm);
        word address = vm_next_word (vm);
        word value = vm_load_word (vm, address);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_RM_RM:
      {
        word dest = vm_next_register_value (vm);
        word address = vm_next_register_value (vm);
        word value = vm_load_word (vm, address);
        vm_store_word (vm, dest, value);
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
        word value = vm_next_register_value (vm);
        vm_push_word (vm, value);
      }
      break;
    case VM_OPERATION_POP:
      {
        word *dest = vm_next_register_address (vm);
        *dest = vm_pop_word (vm);
      }
      break;
    case VM_OPERATION_ADD_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_word (vm);
        *dest = src1 + src2;
      }
      break;
    case VM_OPERATION_ADD_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);
        *dest = src1 + src2;
      }
      break;
    case VM_OPERATION_SUB_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_word (vm);
        *dest = src1 - src2;
      }
      break;
    case VM_OPERATION_SUB_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);
        *dest = src1 - src2;
      }
      break;
    case VM_OPERATION_MUL_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_word (vm);
        *dest = src1 * src2;
      }
      break;
    case VM_OPERATION_MUL_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);
        *dest = src1 * src2;
      }
      break;
    case VM_OPERATION_DIV_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_word (vm);

        vm_assert (src2 != 0, VM_ERROR_ILLEGAL_OPERATION);

        vm->registers[VM_REGISTER_AC] = src1 % src2;
        *dest = src1 / src2;
      }
      break;
    case VM_OPERATION_DIV_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);

        vm_assert (src2 != 0, VM_ERROR_ILLEGAL_OPERATION);

        vm->registers[VM_REGISTER_AC] = src1 % src2;
        *dest = src1 / src2;
      }
      break;
    case VM_OPERATION_CMP_R_I:
      {
        word left = vm_next_register_value (vm);
        word right = vm_next_word (vm);
        vm_compare (vm, left, right);
      }
      break;
    case VM_OPERATION_CMP_R_R:
      {
        word left = vm_next_register_value (vm);
        word right = vm_next_register_value (vm);
        vm_compare (vm, left, right);
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
        word address = vm_next_register_value (vm);
        *vm->ip = address;
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
        word address = vm_next_register_value (vm);
        vm_jump (vm, address, vm->flags.z == 1);
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
        word address = vm_next_register_value (vm);
        vm_jump (vm, address, vm->flags.z == 0);
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
        word address = vm_next_register_value (vm);
        vm_jump (vm, address, vm->flags.c == 1);
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
        word address = vm_next_register_value (vm);
        vm_jump (vm, address, vm->flags.z == 0 && vm->flags.c == 0);
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
        word address = vm_next_register_value (vm);
        vm_jump (vm, address, vm->flags.c == 1 || vm->flags.z == 1);
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
        word address = vm_next_register_value (vm);
        vm_jump (vm, address, vm->flags.c == 0);
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
        word address = vm_next_register_value (vm);
        vm_push_word (vm, *vm->ip);
        *vm->ip = address;
      }
      break;
    case VM_OPERATION_RET:
      *vm->ip = vm_pop_word (vm);
      break;
    case VM_OPERATION_HALT:
      exit (VM_ERROR_NONE);
      break;
    default:
      exit (VM_ERROR_ILLEGAL_OPERATION);
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
  size_t dec = (address >= a) ? a : address;
  size_t inc = (address <= vm->nmemb - 1 - b) ? b : vm->nmemb - 1 - address;

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
  printf ("z=%d c=%d\n", vm->flags.z, vm->flags.c);
}

