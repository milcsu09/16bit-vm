#ifndef VM_H
#define VM_H

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define VM_FMT_BYTE "%02x"
#define VM_FMT_WORD "%04x"
#define VM_UNKNOWN "unknown"

#define VM_WORD_PACK(H, L) ((H) << 8 | (L))
#define VM_WORD_H(W) ((W) >> 8)
#define VM_WORD_L(W) ((W) & 0xFF)

#define VM_ARRAY_SIZE(xs) (sizeof (xs) / sizeof ((xs)[0]))

typedef uint8_t byte;
typedef uint16_t word;

typedef enum
{
  VM_REGISTER_IP,
  VM_REGISTER_SP,
  VM_REGISTER_BP,

  VM_REGISTER_AC,

  VM_REGISTER_R1,
  VM_REGISTER_R2,
  VM_REGISTER_R3,
  VM_REGISTER_R4,
  VM_REGISTER_R5,
  VM_REGISTER_R6,
  VM_REGISTER_R7,
  VM_REGISTER_R8,

  VM_REGISTER_COUNT,
} VM_Register;

static const char *const VM_REGISTER_NAME[] = {
  "ip", "sp", "bp", "ac", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
};

static_assert (VM_ARRAY_SIZE (VM_REGISTER_NAME) == VM_REGISTER_COUNT,
               "insufficient items in VM_REGISTER_NAME");

// VM_OPERATION_*
// "I" - Immediate value (comptime)
// "R" - Register
// "M" - Memory address
typedef enum
{
  VM_OPERATION_NOP,

  VM_OPERATION_MOV_R_I,
  VM_OPERATION_MOV_R_R,
  VM_OPERATION_MOV_R_IM,
  VM_OPERATION_MOV_R_RM,

  VM_OPERATION_MOV_IM_I,
  VM_OPERATION_MOV_IM_R,
  VM_OPERATION_MOV_IM_IM,
  VM_OPERATION_MOV_IM_RM,

  VM_OPERATION_MOV_RM_I,
  VM_OPERATION_MOV_RM_R,
  VM_OPERATION_MOV_RM_IM,
  VM_OPERATION_MOV_RM_RM,

  VM_OPERATION_PUSH_I,
  VM_OPERATION_PUSH_R,
  VM_OPERATION_POP,

  VM_OPERATION_ADD_I,
  VM_OPERATION_ADD_R,

  VM_OPERATION_SUB_I,
  VM_OPERATION_SUB_R,

  VM_OPERATION_MUL_I,
  VM_OPERATION_MUL_R,

  VM_OPERATION_DIV_I,
  VM_OPERATION_DIV_R,

  VM_OPERATION_CMP_R_I,
  VM_OPERATION_CMP_R_R,

  VM_OPERATION_JMP_I,
  VM_OPERATION_JMP_R,

  VM_OPERATION_JEQ_I,
  VM_OPERATION_JEQ_R,

  VM_OPERATION_JNE_I,
  VM_OPERATION_JNE_R,

  VM_OPERATION_JLT_I,
  VM_OPERATION_JLT_R,

  VM_OPERATION_JGT_I,
  VM_OPERATION_JGT_R,

  VM_OPERATION_JLE_I,
  VM_OPERATION_JLE_R,

  VM_OPERATION_JGE_I,
  VM_OPERATION_JGE_R,

  VM_OPERATION_HALT,

  VM_OPERATION_COUNT,
} VM_Operation;

static const char *const VM_OPERATION_NAME[] = {
  "nop",
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
  "halt",
};

static_assert (VM_ARRAY_SIZE (VM_OPERATION_NAME) == VM_OPERATION_COUNT,
               "insufficient items in VM_OPERATION_NAME");

const char *vm_operation_name (VM_Operation operation);

typedef struct
{
  word registers[VM_REGISTER_COUNT];
  word *ip; // -> registers[VM_REGISTER_IP]
  word *sp; // -> registers[VM_REGISTER_SP]
  word *bp; // -> registers[VM_REGISTER_BP]
  byte *memory;

  struct
  {
    byte z : 1;
    byte n : 1;
    byte c : 1;
    byte v : 1;
  } flags;

  word nmemb;
  bool halt;
} VM;

void vm_create (VM *vm, word nmemb);
void vm_destroy (VM *vm);

byte vm_load_byte (VM *vm, word address);
word vm_load_word (VM *vm, word address);
word *vm_load_register (VM *vm, word address);

byte vm_next_byte (VM *vm);
word vm_next_word (VM *vm);
word *vm_next_register (VM *vm);

void vm_store_byte (VM *vm, word address, byte value);
void vm_store_word (VM *vm, word address, word value);

void vm_compare (VM *vm, word left, word right);
void vm_jump (VM *vm, word address, byte condition);

void vm_execute (VM *vm, VM_Operation operation);
void vm_step (VM *vm);

void vm_view_registers (VM *vm);
void vm_view_memory (VM *vm, word address, word a, word b, bool decode);
void vm_view_flags (VM *vm);

#endif // VM_H

