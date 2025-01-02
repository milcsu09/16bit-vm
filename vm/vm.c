#include "vm.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VM_STACK_POINTER_DELTA sizeof (word)

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
  "pusha",
  "popa",
  "add(i)",
  "add(r)",
  "sub(i)",
  "sub(r)",
  "mul(i)",
  "mul(r)",
  "div(i)",
  "div(r)",
  "cmp(i)",
  "cmp(r)",
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
};

static_assert (VM_ARRAY_SIZE (VM_REGISTER_NAME) == VM_REGISTER_COUNT,
               "items not aligned in VM_REGISTER_NAME");

static_assert (VM_ARRAY_SIZE (VM_OPERATION_NAME) == VM_OPERATION_COUNT,
               "items not aligned in VM_OPERATION_NAME");

static_assert (VM_ARRAY_SIZE (VM_ERROR_NAME) == VM_ERROR_COUNT,
               "items not aligned in VM_ERROR_NAME");

static inline char *
vm_module_name (size_t index, size_t n, const char *const xs[n])
{
  return (char *)(index < n ? xs[index] : "invalid");
}

char *
vm_register_name (VM_Register index)
{
  return vm_module_name (index, VM_REGISTER_COUNT, VM_REGISTER_NAME);
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
vm_create (VM *vm)
{
  vm->nmemory = 0x10000; // Default to 64kB
  vm->ndevice = vm->nmemory / VM_DEVICE_BLOCK_SIZE;

  vm->registers[VM_REGISTER_SP] = vm->nmemory - sizeof (word);
  vm->registers[VM_REGISTER_BP] = vm->registers[VM_REGISTER_SP];

  vm->ip = &vm->registers[VM_REGISTER_IP];
  vm->sp = &vm->registers[VM_REGISTER_SP];
  vm->bp = &vm->registers[VM_REGISTER_BP];

  vm->memory = calloc (vm->nmemory, sizeof (byte));
  vm->devices = calloc (vm->ndevice, sizeof (VM_Device *));

  vm->halt = false;
}

void
vm_destroy (VM *vm)
{
  free (vm->memory);
  free (vm->devices);

  vm->memory = NULL;
  vm->devices = NULL;

  vm->nmemory = 0;
  vm->ndevice = 0;
}

void
vm_map_device (VM *vm, VM_Device *device, word start, word end)
{
  start /= VM_DEVICE_BLOCK_SIZE;
  end /= VM_DEVICE_BLOCK_SIZE;

  for (word i = start; i <= end; ++i)
    vm->devices[i] = device;
}

static inline VM_Device *
vm_find_device (VM *vm, word address)
{
  return vm->devices[address / VM_DEVICE_BLOCK_SIZE];
}

byte
vm_load_byte (VM *vm, word address)
{
  VM_Device *device = vm_find_device (vm, address);
  if (device)
    return device->load (vm, device, address);
  return vm->memory[address];
}

word
vm_load_word (VM *vm, word address)
{
  const byte L = vm_load_byte (vm, address + 0);
  const byte H = vm_load_byte (vm, address + 1);
  return VM_WORD_PACK (H, L);
}

word
vm_load_width (VM *vm, word address, bool half)
{
  if (half)
    return vm_load_byte (vm, address);
  else
    return vm_load_word (vm, address);
}

word
vm_load_register_value (VM *vm, word address)
{
  byte index = vm_load_byte (vm, address);
  return vm->registers[index];
}

word *
vm_load_register_address (VM *vm, word address)
{
  byte index = vm_load_byte (vm, address);
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
  const byte L = vm_next_byte (vm);
  const byte H = vm_next_byte (vm);
  return VM_WORD_PACK (H, L);
}

word
vm_next_width (VM *vm, bool half)
{
  if (half)
    return vm_next_byte (vm);
  else
    return vm_next_word (vm);
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
  VM_Device *device = vm_find_device (vm, address);
  if (device)
    device->store (vm, device, address, value);
  else
    vm->memory[address] = value;
}

void
vm_store_word (VM *vm, word address, word value)
{
  const byte L = VM_WORD_L (value);
  const byte H = VM_WORD_H (value);

  vm_store_byte (vm, address + 0, L);
  vm_store_byte (vm, address + 1, H);
}

void
vm_store_width (VM *vm, word address, word value, bool half)
{
  if (half)
    vm_store_byte (vm, address, value);
  else
    vm_store_word (vm, address, value);
}

void
vm_push_byte (VM *vm, byte value)
{
  vm_store_byte (vm, *vm->sp, value);
  *vm->sp -= VM_STACK_POINTER_DELTA;
}

void
vm_push_word (VM *vm, word value)
{
  vm_store_word (vm, *vm->sp, value);
  *vm->sp -= VM_STACK_POINTER_DELTA;
}

byte
vm_pop_byte (VM *vm)
{
  *vm->sp += VM_STACK_POINTER_DELTA;
  return vm_load_byte (vm, *vm->sp);
}

word
vm_pop_word (VM *vm)
{
  *vm->sp += VM_STACK_POINTER_DELTA;
  return vm_load_word (vm, *vm->sp);
}

void
vm_compare (VM *vm, word a, word b)
{
  vm->flags.z = (a == b);
  vm->flags.c = (a < b);
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
  byte half = operation & 0x80;
  byte data = operation & 0x7F;

  switch (data)
    {
    case VM_OPERATION_NONE:
      break;
    case VM_OPERATION_MOV_R_I:
      {
        word *dest = vm_next_register_address (vm);
        word value = vm_next_width (vm, half);
        *dest = value;
      }
      break;
    case VM_OPERATION_MOV_R_R:
      {
        word *dest = vm_next_register_address (vm);
        word value = vm_next_register_value (vm);
        *dest = value;
      }
      break;
    case VM_OPERATION_MOV_R_IM:
      {
        word *dest = vm_next_register_address (vm);
        word address = vm_next_word (vm);
        *dest = vm_load_width (vm, address, half);
      }
      break;
    case VM_OPERATION_MOV_R_RM:
      {
        word *dest = vm_next_register_address (vm);
        word address = vm_next_register_value (vm);
        *dest = vm_load_width (vm, address, half);
      }
      break;
    case VM_OPERATION_MOV_IM_I:
      {
        word dest = vm_next_word (vm);
        word value = vm_next_width (vm, half);
        vm_store_width (vm, dest, value, half);
      }
      break;
    case VM_OPERATION_MOV_IM_R:
      {
        word dest = vm_next_word (vm);
        word value = vm_next_register_value (vm);
        vm_store_width (vm, dest, value, half);
      }
      break;
    case VM_OPERATION_MOV_IM_IM:
      {
        word dest = vm_next_word (vm);
        word address = vm_next_word (vm);
        word value = vm_load_width (vm, address, half);
        vm_store_width (vm, dest, value, half);
      }
      break;
    case VM_OPERATION_MOV_IM_RM:
      {
        word dest = vm_next_word (vm);
        word address = vm_next_register_value (vm);
        word value = vm_load_width (vm, address, half);
        vm_store_width (vm, dest, value, half);
      }
      break;
    case VM_OPERATION_MOV_RM_I:
      {
        word dest = vm_next_register_value (vm);
        word value = vm_next_width (vm, half);
        vm_store_width (vm, dest, value, half);
      }
      break;
    case VM_OPERATION_MOV_RM_R:
      {
        word dest = vm_next_register_value (vm);
        word value = vm_next_register_value (vm);
        vm_store_width (vm, dest, value, half);
      }
      break;
    case VM_OPERATION_MOV_RM_IM:
      {
        word dest = vm_next_register_value (vm);
        word address = vm_next_word (vm);
        word value = vm_load_width (vm, address, half);
        vm_store_width (vm, dest, value, half);
      }
      break;
    case VM_OPERATION_MOV_RM_RM:
      {
        word dest = vm_next_register_value (vm);
        word address = vm_next_register_value (vm);
        word value = vm_load_width (vm, address, half);
        vm_store_width (vm, dest, value, half);
      }
      break;
    case VM_OPERATION_PUSH_I:
      {
        word value = vm_next_width (vm, half);
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
    case VM_OPERATION_PUSHA:
      for (size_t i = VM_REGISTER_R1; i <= VM_REGISTER_R8; ++i)
        vm_push_word (vm, vm->registers[i]);
      break;
    case VM_OPERATION_POPA:
      for (size_t i = VM_REGISTER_R8; i >= VM_REGISTER_R1; --i)
        vm->registers[i] = vm_pop_word (vm);
      break;
    case VM_OPERATION_ADD_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_width (vm, half);
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
        word src2 = vm_next_width (vm, half);
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
        word src2 = vm_next_width (vm, half);
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
        word src2 = vm_next_width (vm, half);
        vm->registers[VM_REGISTER_AC] = src1 % src2;
        *dest = src1 / src2;
      }
      break;
    case VM_OPERATION_DIV_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);
        vm->registers[VM_REGISTER_AC] = src1 % src2;
        *dest = src1 / src2;
      }
      break;
    case VM_OPERATION_CMP_I:
      {
        word a = vm_next_register_value (vm);
        word b = vm_next_width (vm, half);
        vm_compare (vm, a, b);
      }
      break;
    case VM_OPERATION_CMP_R:
      {
        word a = vm_next_register_value (vm);
        word b = vm_next_register_value (vm);
        vm_compare (vm, a, b);
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
      vm->halt = true;
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
vm_view_register (VM *vm, VM_Register index)
{
  word value = vm->registers[index];
  printf ("%s ", vm_register_name (index));

  if (value == 0)
    printf ("....");
  else
    {
      printf (VM_FMT_WORD " (%d)\t", value, value);
      for (int i = 15; i >= 0; i--)
        printf("%d", (value >> i) & 1);
      if (isprint ((char) value))
        printf (" '%c'", value);
    }

  printf ("\n");
}

void
vm_view_memory (VM *vm, word address, word a, word b, bool decode)
{
  size_t above = (address <= vm->nmemory - 1 - a) ? a : vm->nmemory - 1 - address;
  size_t below = (address >= b) ? b : address;

  printf (VM_FMT_WORD " ", address);

  for (size_t i = 0; i < b - below; ++i)
    printf (".. ");

  for (size_t i = address - below; i <= address + above; ++i)
    printf (VM_FMT_BYTE " ", vm->memory[i]);

  for (size_t i = 0; i < a - above; ++i)
    printf (".. ");

  // (VM_FMT_WORD " ") + b * (VM_FMT_BYTE " ")
  printf ("\n%*s^", 5 + b * 3, "");

  if (decode)
    {
      VM_Operation operation = vm->memory[address];
      byte half = operation & 0x80;
      byte data = operation & 0x7F;
      printf ("~ %s%s", vm_operation_name (data), half ? "'" : "");
    }

  printf ("\n");
}

