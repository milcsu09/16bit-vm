#include "vm.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VM_STACK_POINTER_DELTA sizeof (word)

VM_Device vm_device_ram = {
  .read_byte = vm_default_read_byte,
  .read_word = vm_default_read_word,
  .store_byte = vm_default_store_byte,
  .store_word = vm_default_store_word,
  .state = NULL,
};

static const char *const VM_REGISTER_NAME[] = {
  "ip", "sp", "bp", "ac", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
};

static const char *const VM_OPERATION_NAME[] = {
  "NONE",
  "MOV_R_I",
  "MOV_R_R",
  "MOV_R_IM",
  "MOV_R_RM",
  "MOV_IM_I",
  "MOV_IM_R",
  "MOV_IM_IM",
  "MOV_IM_RM",
  "MOV_RM_I",
  "MOV_RM_R",
  "MOV_RM_IM",
  "MOV_RM_RM",
  "MOVB_R_I",
  "MOVB_R_R",
  "MOVB_R_IM",
  "MOVB_R_RM",
  "MOVB_IM_I",
  "MOVB_IM_R",
  "MOVB_IM_IM",
  "MOVB_IM_RM",
  "MOVB_RM_I",
  "MOVB_RM_R",
  "MOVB_RM_IM",
  "MOVB_RM_RM",
  "PUSH_I",
  "PUSH_R",
  "POP",
  "PUSHA",
  "POPA",
  "ADD_I",
  "ADD_R",
  "SUB_I",
  "SUB_R",
  "MUL_I",
  "MUL_R",
  "DIV_I",
  "DIV_R",
  "AND_I",
  "AND_R",
  "OR_I",
  "OR_R",
  "XOR_I",
  "XOR_R",
  "NOT",
  "SHL_I",
  "SHL_R",
  "SHR_I",
  "SHR_R",
  "CMP_I",
  "CMP_R",
  "JMP_I",
  "JMP_R",
  "JEQ_I",
  "JEQ_R",
  "JNE_I",
  "JNE_R",
  "JLT_I",
  "JLT_R",
  "JGT_I",
  "JGT_R",
  "JLE_I",
  "JLE_R",
  "JGE_I",
  "JGE_R",
  "CALL_I",
  "CALL_R",
  "RET",
  "HALT",
  "PRINT_I",
  "PRINT_R",
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

  vm->registers[VM_REGISTER_SP] = vm->nmemory - VM_STACK_POINTER_DELTA;
  vm->registers[VM_REGISTER_BP] = vm->registers[VM_REGISTER_SP];

  vm->ip = &vm->registers[VM_REGISTER_IP];
  vm->sp = &vm->registers[VM_REGISTER_SP];
  vm->bp = &vm->registers[VM_REGISTER_BP];

  vm->memory = calloc (vm->nmemory, sizeof (byte));
  vm->devices = calloc (vm->ndevice, sizeof (VM_Device *));

  vm->halt = false;

  vm_map_device (vm, &vm_device_ram, 0, vm->nmemory - 1);
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
vm_load (VM *vm, byte *memory, size_t nmemory)
{
  if (nmemory > vm->nmemory)
    nmemory = vm->nmemory;

  memcpy (vm->memory, memory, nmemory);
}

bool
vm_load_file (VM *vm, const char *path)
{
  FILE *file = fopen (path, "rb");

  if (!file)
    {
      perror ("Failed to open file");
      return false;
    }

  if (fseek (file, 0, SEEK_END) != 0)
    {
      perror ("Failed to seek file");
      fclose (file);
      return false;
    }

  long nmemory = ftell (file);

  if (nmemory < 0)
    {
      perror ("Failed to tell file size");
      fclose (file);
      return false;
    }

  rewind(file);

  byte *memory = calloc (nmemory, sizeof (byte));

  size_t nread = fread (memory, 1, nmemory, file);

  if (nread != (size_t)nmemory)
    {
      perror ("Failed to read file");
      free (memory);
      fclose (file);
      return false;
    }

  vm_load (vm, memory, nmemory);

  free (memory);
  fclose (file);

  return true;
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
vm_default_read_byte (VM *vm, VM_Device *device, word address)
{
  (void)device;
  return vm->memory[address];
}

word
vm_default_read_word (VM *vm, VM_Device *device, word address)
{
  (void)device;
  const byte L = vm_read_byte (vm, address + 0);
  const byte H = vm_read_byte (vm, address + 1);
  return VM_WORD_PACK (H, L);
}

void
vm_default_store_byte (VM *vm, VM_Device *device, word address, byte value)
{
  (void)device;
  vm->memory[address] = value;
}

void
vm_default_store_word (VM *vm, VM_Device *device, word address, word value)
{
  (void)device;
  const byte L = VM_WORD_L (value);
  const byte H = VM_WORD_H (value);
  vm_store_byte (vm, address + 0, L);
  vm_store_byte (vm, address + 1, H);
}

byte
vm_read_byte (VM *vm, word address)
{
  VM_Device *device = vm_find_device (vm, address);
  return device->read_byte (vm, device, address);
}

word
vm_read_word (VM *vm, word address)
{
  VM_Device *device = vm_find_device (vm, address);
  return device->read_word (vm, device, address);
}

/*
word
vm_read_width (VM *vm, word address, bool full)
{
  if (full)
    return vm_read_word (vm, address);
  else
    return vm_read_byte (vm, address);
}
*/

word
vm_read_register_value (VM *vm, word address)
{
  byte index = vm_read_byte (vm, address);
  return vm->registers[index];
}

word *
vm_read_register_address (VM *vm, word address)
{
  byte index = vm_read_byte (vm, address);
  return &vm->registers[index];
}

byte
vm_next_byte (VM *vm)
{
  return vm_read_byte (vm, (*vm->ip)++);
}

word
vm_next_word (VM *vm)
{
  const byte L = vm_next_byte (vm);
  const byte H = vm_next_byte (vm);
  return VM_WORD_PACK (H, L);
}

/*
word
vm_next_width (VM *vm, bool full)
{
  if (full)
    return vm_next_word (vm);
  else
    return vm_next_byte (vm);
}
*/

word
vm_next_register_value (VM *vm)
{
  return vm_read_register_value (vm, (*vm->ip)++);
}

word *
vm_next_register_address (VM *vm)
{
  return vm_read_register_address (vm, (*vm->ip)++);
}

void
vm_store_byte (VM *vm, word address, byte value)
{
  VM_Device *device = vm_find_device (vm, address);
  device->store_byte (vm, device, address, value);
}

void
vm_store_word (VM *vm, word address, word value)
{
  VM_Device *device = vm_find_device (vm, address);
  device->store_word (vm, device, address, value);
}

/*
void
vm_store_width (VM *vm, word address, word value, bool full)
{
  if (full)
    vm_store_word (vm, address, value);
  else
    vm_store_byte (vm, address, value);
}
*/

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
  return vm_read_byte (vm, *vm->sp);
}

word
vm_pop_word (VM *vm)
{
  *vm->sp += VM_STACK_POINTER_DELTA;
  return vm_read_word (vm, *vm->sp);
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
  // byte full = operation & 0x80;
  // byte data = operation & 0x7F;

  switch (operation)
    {
    case VM_OPERATION_NONE:
      break;
    case VM_OPERATION_MOV_R_I:
      {
        word *dest = vm_next_register_address (vm);
        word value = vm_next_word (vm);
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
        *dest = vm_read_word (vm, address);
      }
      break;
    case VM_OPERATION_MOV_R_RM:
      {
        word *dest = vm_next_register_address (vm);
        word address = vm_next_register_value (vm);
        *dest = vm_read_word (vm, address);
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
        word value = vm_read_word (vm, address);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_IM_RM:
      {
        word dest = vm_next_word (vm);
        word address = vm_next_register_value (vm);
        word value = vm_read_word (vm, address);
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
        word dest = vm_next_register_value (vm);
        word value = vm_next_register_value (vm);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_RM_IM:
      {
        word dest = vm_next_register_value (vm);
        word address = vm_next_word (vm);
        word value = vm_read_word (vm, address);
        vm_store_word (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOV_RM_RM:
      {
        word dest = vm_next_register_value (vm);
        word address = vm_next_register_value (vm);
        word value = vm_read_word (vm, address);
        vm_store_word (vm, dest, value);
      }
      break;


    case VM_OPERATION_MOVB_R_I:
      {
        word *dest = vm_next_register_address (vm);
        byte value = vm_next_byte (vm);
        *dest = value;
      }
      break;
    case VM_OPERATION_MOVB_R_R:
      {
        word *dest = vm_next_register_address (vm);
        byte value = vm_next_register_value (vm);
        *dest = value;
      }
      break;
    case VM_OPERATION_MOVB_R_IM:
      {
        word *dest = vm_next_register_address (vm);
        word address = vm_next_word (vm);
        *dest = vm_read_byte (vm, address);
      }
      break;
    case VM_OPERATION_MOVB_R_RM:
      {
        word *dest = vm_next_register_address (vm);
        word address = vm_next_register_value (vm);
        *dest = vm_read_byte (vm, address);
      }
      break;
    case VM_OPERATION_MOVB_IM_I:
      {
        word dest = vm_next_word (vm);
        byte value = vm_next_byte (vm);
        vm_store_byte (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOVB_IM_R:
      {
        word dest = vm_next_word (vm);
        byte value = vm_next_register_value (vm);
        vm_store_byte (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOVB_IM_IM:
      {
        word dest = vm_next_word (vm);
        word address = vm_next_word (vm);
        byte value = vm_read_byte (vm, address);
        vm_store_byte (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOVB_IM_RM:
      {
        word dest = vm_next_word (vm);
        word address = vm_next_register_value (vm);
        byte value = vm_read_byte (vm, address);
        vm_store_byte (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOVB_RM_I:
      {
        word dest = vm_next_register_value (vm);
        byte value = vm_next_byte (vm);
        vm_store_byte (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOVB_RM_R:
      {
        word dest = vm_next_register_value (vm);
        byte value = vm_next_register_value (vm);
        vm_store_byte (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOVB_RM_IM:
      {
        word dest = vm_next_register_value (vm);
        word address = vm_next_word (vm);
        byte value = vm_read_byte (vm, address);
        vm_store_byte (vm, dest, value);
      }
      break;
    case VM_OPERATION_MOVB_RM_RM:
      {
        word dest = vm_next_register_value (vm);
        word address = vm_next_register_value (vm);
        byte value = vm_read_byte (vm, address);
        vm_store_byte (vm, dest, value);
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
    case VM_OPERATION_PUSHA:
      {
        for (size_t i = VM_REGISTER_R1; i <= VM_REGISTER_R8; ++i)
          vm_push_word (vm, vm->registers[i]);
      }
      break;
    case VM_OPERATION_POPA:
      {
        for (size_t i = VM_REGISTER_R8; i >= VM_REGISTER_R1; --i)
          vm->registers[i] = vm_pop_word (vm);
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
    case VM_OPERATION_AND_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_word (vm);
        *dest = src1 & src2;
      }
      break;
    case VM_OPERATION_AND_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);
        *dest = src1 & src2;
      }
      break;
    case VM_OPERATION_OR_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_word (vm);
        *dest = src1 | src2;
      }
      break;
    case VM_OPERATION_OR_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);
        *dest = src1 | src2;
      }
      break;
    case VM_OPERATION_XOR_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_word (vm);
        *dest = src1 ^ src2;
      }
      break;
    case VM_OPERATION_XOR_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);
        *dest = src1 ^ src2;
      }
      break;
    case VM_OPERATION_NOT:
      {
        word *dest = vm_next_register_address (vm);
        word value = vm_next_register_value (vm);
        *dest = ~value;
      }
      break;
    case VM_OPERATION_SHL_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_word (vm);
        *dest = src1 << src2;
      }
      break;
    case VM_OPERATION_SHL_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);
        *dest = src1 << src2;
      }
      break;
    case VM_OPERATION_SHR_I:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_word (vm);
        *dest = src1 >> src2;
      }
      break;
    case VM_OPERATION_SHR_R:
      {
        word *dest = vm_next_register_address (vm);
        word src1 = vm_next_register_value (vm);
        word src2 = vm_next_register_value (vm);
        *dest = src1 >> src2;
      }
      break;
    case VM_OPERATION_CMP_I:
      {
        word a = vm_next_register_value (vm);
        word b = vm_next_word (vm);
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
        vm_jump (vm, address, vm->flags.z == 1 || vm->flags.c == 1);
      }
      break;
    case VM_OPERATION_JLE_R:
      {
        word address = vm_next_register_value (vm);
        vm_jump (vm, address, vm->flags.z == 1 || vm->flags.c == 1);
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
    case VM_OPERATION_PRINT_I:
      {
        word value = vm_next_word (vm);
        printf ("%d\n", value);
      }
      break;
    case VM_OPERATION_PRINT_R:
      {
        word value = vm_next_register_value (vm);
        printf ("%d\n", value);
      }
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
      printf (" %.8f", (value / (float)(1 << 8)));
    }

  printf ("\n");
}

void
vm_view_memory (VM *vm, word address, word b, word a, int decode)
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
  printf ("\n%*s", 5 + b * 3, "");

  switch (decode)
    {
    case 1: // Decode operation
      {
        printf ("^");
        VM_Operation operation = vm->memory[address];
        // byte full = operation & 0x80;
        // byte data = operation & 0x7F;
        printf ("~ %s", vm_operation_name (operation));
      }
      break;
    case 2: // Decode ASCII string
      {
        for (size_t i = address; i <= address + above; ++i)
          {
            char c = vm->memory[i];
            if (isprint (c))
              printf ("%c  ", c);
            else
              printf (".  ");
          }
      }
      break;
    default:
      printf ("^");
      break;
    }

  printf ("\n");
}

