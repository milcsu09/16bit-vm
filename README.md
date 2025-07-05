# 16-bit Virtual Machine

16-bit Virtual Machine written in C

## Details

### Registers

| Register | Description                                                     |
|----------|-----------------------------------------------------------------|
| `IP`     | Instruction pointer                                             |
| `SP`     | Stack pointer                                                   |
| `BP`     | Base pointer                                                    |
| `AC`     | Accumulator                                                     |
| `R1`     | General purpose                                                 |
| `R2`     | General purpose                                                 |
| `R3`     | General purpose                                                 |
| `R4`     | General purpose                                                 |
| `R5`     | General purpose                                                 |
| `R6`     | General purpose                                                 |
| `R7`     | General purpose                                                 |
| `R8`     | General purpose                                                 |

### Operations

#### Operand notations
| Notation | Description                                                     |
|----------|-----------------------------------------------------------------|
| `I`      | Immediate value                                                 |
| `R`      | Register                                                        |
| `IM`     | Immediate memory (address)                                      |
| `RM`     | Register memory (address)                                       |

#### Operation table
| Code   | Instruction  | Operands          | Description                                            |
|--------|--------------|-------------------|--------------------------------------------------------|
| `0x00` | `NONE`       | -                 | None                                                   |
| `0x01` | `MOV_R_I`    | `R1`, `I1`        | Move `16` bits from `I1` to `R1`                       |
| `0x02` | `MOV_R_R`    | `R1`, `R2`        | Move `16` bits from `R2` to `R1`                       |
| `0x03` | `MOV_R_IM`   | `R1`, `IM1`       | Move `16` bits from `IM1` to `R1`                      |
| `0x04` | `MOV_R_RM`   | `R1`, `RM1`       | Move `16` bits from `RM1` to `R1`                      |
| `0x05` | `MOV_IM_I`   | `IM1`, `I1`       | Move `16` bits from `I1` to `IM1`                      |
| `0x06` | `MOV_IM_R`   | `IM1`, `R1`       | Move `16` bits from `R1` to `IM1`                      |
| `0x07` | `MOV_IM_IM`  | `IM1`, `IM2`      | Move `16` bits from `IM2` to `IM1`                     |
| `0x08` | `MOV_IM_RM`  | `IM1`, `RM1`      | Move `16` bits from `RM1` to `IM1`                     |
| `0x09` | `MOV_RM_I`   | `RM1`, `I1`       | Move `16` bits from `I1` to `RM1`                      |
| `0x0a` | `MOV_RM_R`   | `RM1`, `R1`       | Move `16` bits from `R1` to `RM1`                      |
| `0x0b` | `MOV_RM_IM`  | `RM1`, `IM1`      | Move `16` bits from `IM1` to `RM1`                     |
| `0x0c` | `MOV_RM_RM`  | `RM1`, `RM2`      | Move `16` bits from `RM2` to `RM2`                     |
| `0x0d` | `MOVB_R_I`   | `R1`, `I1`        | Move `8` bits from `I1` to `R1`                        |
| `0x0e` | `MOVB_R_R`   | `R1`, `R2`        | Move `8` bits from `R2` to `R1`                        |
| `0x0f` | `MOVB_R_IM`  | `R1`, `IM1`       | Move `8` bits from `IM1` to `R1`                       |
| `0x10` | `MOVB_R_RM`  | `R1`, `RM1`       | Move `8` bits from `RM1` to `R1`                       |
| `0x11` | `MOVB_IM_I`  | `IM1`, `I1`       | Move `8` bits from `I1` to `IM1`                       |
| `0x12` | `MOVB_IM_R`  | `IM1`, `R1`       | Move `8` bits from `R1` to `IM1`                       |
| `0x13` | `MOVB_IM_IM` | `IM1`, `IM2`      | Move `8` bits from `IM2` to `IM1`                      |
| `0x14` | `MOVB_IM_RM` | `IM1`, `RM1`      | Move `8` bits from `RM1` to `IM1`                      |
| `0x15` | `MOVB_RM_I`  | `RM1`, `I1`       | Move `8` bits from `I1` to `RM1`                       |
| `0x16` | `MOVB_RM_R`  | `RM1`, `R1`       | Move `8` bits from `R1` to `RM1`                       |
| `0x17` | `MOVB_RM_IM` | `RM1`, `IM1`      | Move `8` bits from `IM1` to `RM1`                      |
| `0x18` | `MOVB_RM_RM` | `RM1`, `RM2`      | Move `8` bits from `RM2` to `RM2`                      |
| `0x19` | `PUSH_I`     | `I1`              | Push `I1` to stack                                     |
| `0x1a` | `PUSH_R`     | `R1`              | Push `R1` to stack                                     |
| `0x1b` | `POP`        | `R1`              | Pop `16` bits from stack, store in `R1`                |
| `0x1c` | `PUSHA`      | -                 | Push all general purpose registers to stack            |
| `0x1d` | `POPA`       | -                 | Pop all general purpose registers from stack           |
| `0x1e` | `ADD_I`      | `R1`, `R2`, `I1`  | Store `R2 + I1` to `R1`                                |
| `0x1f` | `ADD_R`      | `R1`, `R2`, `R3`  | Store `R2 + R3` to `R1`                                |
| `0x20` | `SUB_I`      | `R1`, `R2`, `I1`  | Store `R2 - I1` to `R1`                                |
| `0x21` | `SUB_R`      | `R1`, `R2`, `R3`  | Store `R2 - R3` to `R1`                                |
| `0x22` | `MUL_I`      | `R1`, `R2`, `I1`  | Store `R2 * I1` to `R1`                                |
| `0x23` | `MUL_R`      | `R1`, `R2`, `R3`  | Store `R2 * R3` to `R1`                                |
| `0x24` | `DIV_I`      | `R1`, `R2`, `I1`  | Store `R2 / I1` to `R1` and `R2 % I1` to `AC`          |
| `0x25` | `DIV_R`      | `R1`, `R2`, `R3`  | Store `R2 / R3` to `R1` and `R2 % R3` to `AC`          |
| `0x26` | `AND_I`      | `R1`, `R2`, `I1`  | Store `R2 & I1` to `R1`                                |
| `0x27` | `AND_R`      | `R1`, `R2`, `R3`  | Store `R2 & R3` to `R1`                                |
| `0x28` | `OR_I`       | `R1`, `R2`, `I1`  | Store `R2 \| I1` to `R1`                               |
| `0x29` | `OR_R`       | `R1`, `R2`, `R3`  | Store `R2 \| R3` to `R1`                               |
| `0x2a` | `XOR_I`      | `R1`, `R2`, `I1`  | Store `R2 ^ I1` to `R1`                                |
| `0x2b` | `XOR_R`      | `R1`, `R2`, `R3`  | Store `R2 ^ R3` to `R1`                                |
| `0x2c` | `NOT`        | `R1`, `R2`        | Store `~R2` to `R1`                                    |
| `0x2d` | `SHL_I`      | `R1`, `R2`, `I1`  | Store `R2 << I1` to `R1`                               |
| `0x2e` | `SHL_R`      | `R1`, `R2`, `R3`  | Store `R2 << R3` to `R1`                               |
| `0x2f` | `SHR_I`      | `R1`, `R2`, `I1`  | Store `R2 >> I1` to `R1`                               |
| `0x30` | `SHR_R`      | `R1`, `R2`, `R3`  | Store `R2 >> R3` to `R1`                               |
| `0x31` | `CMP_I`      | `R1`, `I1`        | Set comparison flags                                   |
| `0x32` | `CMP_R`      | `R1`, `R2`        | Set comparison flags                                   |
| `0x33` | `JMP_I`      | `I1`              | Set `IP` to `I1`                                       |
| `0x34` | `JMP_R`      | `R1`              | Set `IP` to `R1`                                       |
| `0x35` | `JEQ_I`      | `I1`              | Set `IP` to `I1` if equal (`Z`=`1`)                    |
| `0x36` | `JEQ_R`      | `R1`              | Set `IP` to `R1` if equal (`Z`=`1`)                    |
| `0x37` | `JNE_I`      | `I1`              | Set `IP` to `I1` if not equal (`Z`=`0`)                |
| `0x38` | `JNE_R`      | `R1`              | Set `IP` to `R1` if not equal (`Z`=`0`)                |
| `0x39` | `JLT_I`      | `I1`              | Set `IP` to `I1` if less (`C`=`1`)                     |
| `0x3a` | `JLT_R`      | `R1`              | Set `IP` to `R1` if less (`C`=`1`)                     |
| `0x3b` | `JGT_I`      | `I1`              | Set `IP` to `I1` if greater (`Z`=`0` and `C`=`0`)      |
| `0x3c` | `JGT_R`      | `R1`              | Set `IP` to `R1` if greater (`Z`=`0` and `C`=`0`)      |
| `0x3d` | `JLE_I`      | `I1`              | Set `IP` to `I1` if less or equal (`Z`=`1` or `C`=`1`) |
| `0x3e` | `JLE_R`      | `R1`              | Set `IP` to `R1` if less or equal (`Z`=`1` or `C`=`1`) |
| `0x3f` | `JGE_I`      | `I1`              | Set `IP` to `I1` if greater or equal (`C`=`0`)         |
| `0x40` | `JGE_R`      | `R1`              | Set `IP` to `R1` if greater or equal (`C`=`0`)         |
| `0x41` | `CALL_I`     | `I1`              | Push `IP` to stack, set `IP` to `I1`                   |
| `0x42` | `CALL_R`     | `R1`              | Push `IP` to stack, set `IP` to `R1`                   |
| `0x43` | `RET`        | -                 | Pop `16` bits from stack, store in `IP`                |
| `0x44` | `HALT`       | -                 | Halt execution                                         |
| `0x45` | *`PRINT_I`   | `I1`              | Print value of `I1` to `stdout`                        |
| `0x46` | *`PRINT_R`   | `IR`              | Print value of `R1` to `stdout`                        |

_* Might be modified or removed_

