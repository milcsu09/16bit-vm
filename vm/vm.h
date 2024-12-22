#ifndef VM_H
#define VM_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define VM_FMT_BYTE "%02x"
#define VM_FMT_WORD "%04x"

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

typedef enum
{
  VM_OPERATION_NONE,
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
  VM_OPERATION_CMP_I,
  VM_OPERATION_CMP_R,
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
  VM_OPERATION_CALL_I,
  VM_OPERATION_CALL_R,
  VM_OPERATION_RET,
  VM_OPERATION_HALT,
  VM_OPERATION_COUNT,
} VM_Operation;

typedef enum
{
  VM_ERROR_NONE,
  VM_ERROR_ILLEGAL_OPERATION,
  VM_ERROR_COUNT,
} VM_Error;

typedef struct
{
  word registers[VM_REGISTER_COUNT];
  word *ip;
  word *sp;
  word *bp;

  byte *memory;
  size_t nmemb;

  struct
  {
    byte z : 1;
    byte c : 1;
  } flags;
} VM;

char *vm_register_name (VM_Register index);
char *vm_operation_name (VM_Operation index);
char *vm_error_name (VM_Error index);

void vm_create (VM *vm, size_t nmemb);
void vm_destroy (VM *vm);

byte vm_load_byte (VM *vm, word address);
word vm_load_word (VM *vm, word address);

word vm_load_register_value (VM *vm, word address);
word *vm_load_register_address (VM *vm, word address);

byte vm_next_byte (VM *vm);
word vm_next_word (VM *vm);

word vm_next_register_value (VM *vm);
word *vm_next_register_address (VM *vm);

void vm_store_byte (VM *vm, word address, byte value);
void vm_store_word (VM *vm, word address, word value);

void vm_push_byte (VM *vm, byte value);
void vm_push_word (VM *vm, word value);

byte vm_pop_byte (VM *vm);
word vm_pop_word (VM *vm);

void vm_compare (VM *vm, word a, word b);
void vm_jump (VM *vm, word address, bool condition);

void vm_execute (VM *vm, VM_Operation operation);
void vm_step (VM *vm);

void vm_view_register (VM *vm, VM_Register index);
void vm_view_memory (VM *vm, word address, word a, word b, bool decode);

#endif // VM_H

