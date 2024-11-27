#!/usr/bin/python3

import sys
import argparse as ap
from typing import *
from dataclasses import dataclass
from enum import IntEnum, Enum, auto, unique

# Type aliases for clarity
Location = Tuple[str, int]
Line = Tuple[int, str]

# Constants
MACRO_PREFIX = "%"

MACRO_DEFINE = MACRO_PREFIX + "define"
MACRO_END = MACRO_PREFIX + "end"

FRAGMENT_LABEL = "."
FRAGMENT_MEMORY = "@"


@unique
class TokenType(Enum):
    DIRECTIVE = auto()
    IMMEDIATE = auto()
    SYMBOL = auto()
    LABEL = auto()
    MEMORY = auto()


@dataclass
class Token:
    typ: TokenType
    loc: Location
    val: Any  # Value depends on token type (int, str, etc.)

    def __repr__(self) -> str:
        if not isinstance(self.val, Token):
            return f"{self.val}"
        return f"{self.typ.name} -> {self.val}"


@unique
class DirectiveType(Enum):
    DW = auto()


DIRECTIVE_NAMES = {item.name.lower(): item for item in DirectiveType}
DIRECTIVE_TYPES = {item: item.name.lower() for item in DirectiveType}


@unique
class OperationType(IntEnum):
    NOP = 0

    MOV_R_I = 1
    MOV_R_R = 2
    MOV_R_IM = 3
    MOV_R_RM = 4

    MOV_IM_I = 5
    MOV_IM_R = 6
    MOV_IM_IM = 7
    MOV_IM_RM = 8

    MOV_RM_I = 9
    MOV_RM_R = 10
    MOV_RM_IM = 11
    MOV_RM_RM = 12

    PUSH_I = 13
    PUSH_R = 14
    POP = 15

    ADD_I = 16
    ADD_R = 17
    SUB_I = 18
    SUB_R = 19
    MUL_I = 20
    MUL_R = 21
    DIV_I = 22
    DIV_R = 23

    HALT = 24

    # Custom OperationType, not present withing bytecode
    DIRECTIVE = -1


@dataclass
class Operation:
    typ: OperationType
    loc: Location
    val: List[Any]  # Operands

    def __repr__(self) -> str:
        return f"({self.typ}: {self.val})"


def report_message(typ: str, msg: str, loc: Location) -> None:
    """Reports a message to stderr"""
    file, line = loc
    sys.stderr.write(f"{file} [{line}]: ")
    sys.stderr.write(f"{typ}: {msg}\n")


def report_error(msg: str, loc: Location) -> None:
    """Reports an error to stderr"""
    report_message("ERROR", msg, loc)


def report_warning(msg: str, loc: Location) -> None:
    """Reports a warning to stderr"""
    report_message("WARNING", msg, loc)


def report_note(msg: str, loc: Location) -> None:
    """Reports a note to stderr"""
    report_message("NOTE", msg, loc)


def enumerate_lines(text: str) -> List[Line]:
    """Splits text into numbered lines"""
    return [(i + 1, line) for i, line in enumerate(text.splitlines())]


def preprocess_lines(lines: List[Line], file: str) -> List[Line]:
    """Preprocess lines"""
    macros: Dict[str, List[Line]] = {}
    macro_name: Optional[str] = None
    macro_buffer: List[Line] = []
    macro_begin: Optional[Location] = None
    result: List[Line] = []

    def handle_macro_define(fragments: List[str], loc: Location) -> None:
        nonlocal macro_name, macro_begin
        if macro_name:
            report_error(f"{MACRO_DEFINE} inside {MACRO_DEFINE}", loc)
            exit(1)
        if len(fragments) != 2:
            report_error(f"{MACRO_DEFINE} requires 1 fragment", loc)
            exit(1)
        macro_name = fragments[1]
        macro_begin = loc
        if macro_name in macros:
            report_error(f"redefinition of {macro_name}", loc)
            exit(1)

    def handle_macro_end(fragments: List[str], loc: Location) -> None:
        nonlocal macro_name, macro_buffer
        if not macro_name:
            report_error(f"{MACRO_END} without {MACRO_DEFINE}", loc)
            exit(1)
        if len(fragments) != 1:
            report_error(f"{MACRO_END} requires 0 fragment", loc)
            exit(1)
        macros[macro_name] = macro_buffer
        macro_name, macro_buffer = None, []

    def handle_macro_expand(name: str, loc: Location) -> None:
        if macro_name:
            report_error(f"macro expansion inside {MACRO_DEFINE}", loc)
            exit(1)
        if not name:
            report_error(f"{MACRO_PREFIX} requires 1 fragment", loc)
            exit(1)
        if name not in macros:
            report_error(f"undefined macro {name}", loc)
            exit(1)
        result.extend(macros[name])

    for i, line in lines:
        loc: Location = (file, i)
        stripped = line.strip()
        fragments = stripped.split()

        if stripped.startswith(MACRO_END):
            handle_macro_end(fragments, loc)
        elif stripped.startswith(MACRO_DEFINE):
            handle_macro_define(fragments, loc)
        elif stripped.startswith(MACRO_PREFIX):
            handle_macro_expand(fragments[0][1:], loc)
        elif macro_name:
            macro_buffer.append((i, line))
        else:
            result.append((i, line))

    if macro_name:
        report_error(f"{MACRO_DEFINE} without {MACRO_END}",
                     cast(Location, macro_begin))
        exit(1)

    return result


def fragment_to_immediate(fragment: str, loc: Location) -> int:
    """Converts a fragment to an immediate value"""
    try:
        base: int | str
        base, value = 10, fragment
        if "#" in fragment:
            base, value = fragment.split("#", maxsplit=1)
            base = int(base)
        return int(value, base) & 0xffff
    except ValueError:
        report_error(f"invalid immediate value {fragment}", loc)
        exit(1)


def fragment_to_token(fragment: str, loc: Location) -> Token:
    """Converts a fragment to a token."""
    if fragment in DIRECTIVE_NAMES:
        return Token(TokenType.DIRECTIVE, loc, DIRECTIVE_NAMES[fragment])
    if fragment[0].isdigit():
        return Token(TokenType.IMMEDIATE, loc,
                     fragment_to_immediate(fragment, loc))
    if fragment.startswith(FRAGMENT_LABEL):
        return Token(TokenType.LABEL, loc, fragment[1:])
    if fragment.startswith(FRAGMENT_MEMORY):
        return Token(TokenType.MEMORY, loc,
                     fragment_to_token(fragment[1:], loc))
    return Token(TokenType.SYMBOL, loc, fragment)


def tokenize_lines(lines: List[Line], file: str) -> List[List[Token]]:
    """Tokenize lines"""
    return [[
        fragment_to_token(fragment, (file, i)) for fragment in line.split()
    ] for i, line in lines if line]


def token_expand_amount(token: Token) -> int:
    """Amount of bytes a token will expand to during pass3"""
    match token.typ:
        case TokenType.DIRECTIVE:
            return 0
        case TokenType.SYMBOL:
            return 1
        case TokenType.IMMEDIATE | TokenType.LABEL:
            return 2
        case TokenType.MEMORY:  # MEMORY stores a token; expand that
            return token_expand_amount(cast(Token, token.val))


def pass1(lines: List[List[Token]]) -> Dict[str, int]:
    """Traverse lines, generate labels"""
    labels: Dict[str, int] = {}
    ip = 0

    for tokens in lines:
        operation, *operands = tokens
        loc: Location = operation.loc

        if operation.typ == TokenType.LABEL:
            if operands != []:
                report_error(f"label requires 0 operands", loc)
                exit(1)
            if operation.val in labels:
                report_error(f"redefinition of {operation.val}", loc)
                exit(1)
            labels[cast(str, operation.val)] = ip
            continue

        ip += sum([token_expand_amount(token) for token in tokens])

    return labels


def operands_to_types(operands: List[Token]) -> List[TokenType]:
    return [token.typ for token in operands]


def operands_with_labels(operands: List[Token], loc: Location,
                         labels: Dict[str, int]) -> List[Token]:

    def get_label(key: str) -> int:
        if key not in labels:
            report_error(f"invalid label {key}", loc)
            exit(1)
        return labels[key]

    def make_token(key: str, loc: Location) -> Token:
        return Token(TokenType.IMMEDIATE, loc, get_label(key))

    result = operands.copy()
    for i, operand in enumerate(result):
        if operand.typ == TokenType.LABEL:
            result[i] = make_token(operand.val, operand.loc)
        if operand.typ == TokenType.MEMORY and operand.val.typ == TokenType.LABEL:
            result[i].val = make_token(operand.val.val, operand.val.loc)

    return result


def no_overload(operation: str, operands: List[Token],
                loc: Location) -> NoReturn:
    report_error(f"no possible overload for {operation}", loc)
    if operands:
        report_note(f"got {', '.join([str(item) for item in operands])}", loc)
    exit(1)


def translate_dw(operands: List[Token], loc: Location) -> Operation:
    match operands_to_types(operands):
        case [TokenType.IMMEDIATE]:
            a = [DirectiveType.DW, *operands]
            return Operation(OperationType.DIRECTIVE, loc, a)

    no_overload("dw", operands, loc)


def translate_mov(operands: List[Token], loc: Location) -> Operation:
    """All overload for 'mov'"""

    match operands_to_types(operands):
        case [TokenType.SYMBOL, TokenType.IMMEDIATE]:
            return Operation(OperationType.MOV_R_I, loc, operands)
        case [TokenType.SYMBOL, TokenType.SYMBOL]:
            return Operation(OperationType.MOV_R_R, loc, operands)
        case [TokenType.SYMBOL, TokenType.MEMORY]:
            match operands[1].val.typ:
                case TokenType.IMMEDIATE:
                    return Operation(OperationType.MOV_R_IM, loc, operands)
                case TokenType.SYMBOL:
                    return Operation(OperationType.MOV_R_RM, loc, operands)
        case [TokenType.MEMORY, TokenType.IMMEDIATE]:
            match operands[0].val.typ:
                case TokenType.IMMEDIATE:
                    return Operation(OperationType.MOV_IM_I, loc, operands)
                case TokenType.SYMBOL:
                    return Operation(OperationType.MOV_RM_I, loc, operands)
        case [TokenType.MEMORY, TokenType.SYMBOL]:
            match operands[0].val.typ:
                case TokenType.IMMEDIATE:
                    return Operation(OperationType.MOV_IM_R, loc, operands)
                case TokenType.SYMBOL:
                    return Operation(OperationType.MOV_RM_R, loc, operands)
        case [TokenType.MEMORY, TokenType.MEMORY]:
            match (operands[0].val.typ, operands[1].val.typ):
                case (TokenType.IMMEDIATE, TokenType.IMMEDIATE):
                    return Operation(OperationType.MOV_IM_IM, loc, operands)
                case (TokenType.SYMBOL, TokenType.IMMEDIATE):
                    return Operation(OperationType.MOV_RM_IM, loc, operands)
                case (TokenType.IMMEDIATE, TokenType.SYMBOL):
                    return Operation(OperationType.MOV_IM_RM, loc, operands)
                case (TokenType.SYMBOL, TokenType.SYMBOL):
                    return Operation(OperationType.MOV_RM_RM, loc, operands)

    no_overload("mov", operands, loc)


def translate_push(operands: List[Token], loc: Location) -> Operation:
    match operands_to_types(operands):
        case [TokenType.IMMEDIATE]:
            return Operation(OperationType.PUSH_I, loc, operands)
        case [TokenType.SYMBOL]:
            return Operation(OperationType.PUSH_R, loc, operands)

    no_overload("push", operands, loc)


def translate_pop(operands: List[Token], loc: Location) -> Operation:
    match operands_to_types(operands):
        case [TokenType.SYMBOL]:
            return Operation(OperationType.POP, loc, operands)

    no_overload("pop", operands, loc)


def translate_binary(operands: List[Token], loc: Location, operation: str,
                     types: List[OperationType]) -> Operation:
    match operands_to_types(operands):
        case [TokenType.SYMBOL, TokenType.SYMBOL, TokenType.IMMEDIATE]:
            return Operation(types[0], loc, operands)
        case [TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL]:
            return Operation(types[1], loc, operands)

    no_overload(operation, operands, loc)


def pass2(lines: List[List[Token]], labels: Dict[str, int]) -> List[Operation]:
    """Translate lines, generate operations"""
    operations: List[Operation] = []

    for tokens in lines:
        operation, *operands = tokens
        loc: Location = operation.loc

        operands = operands_with_labels(operands, loc, labels)

        if operation.typ == TokenType.DIRECTIVE:
            if operation.val == DirectiveType.DW:
                operations.append(translate_dw(operands, loc))
                continue

        # Skip non-operations
        if operation.typ != TokenType.SYMBOL:
            continue

        match operation.val:
            case "nop":
                if len(operands) != 0:
                    no_overload("nop", operands, loc)
                operations.append(Operation(OperationType.NOP, loc, []))
            case "mov":
                operations.append(translate_mov(operands, loc))
            case "push":
                operations.append(translate_push(operands, loc))
            case "pop":
                operations.append(translate_pop(operands, loc))
            case "add":
                operations.append(
                    translate_binary(
                        operands, loc, "add",
                        [OperationType.ADD_I, OperationType.ADD_R]))
            case "sub":
                operations.append(
                    translate_binary(
                        operands, loc, "sub",
                        [OperationType.SUB_I, OperationType.SUB_R]))
            case "mul":
                operations.append(
                    translate_binary(
                        operands, loc, "mul",
                        [OperationType.MUL_I, OperationType.MUL_R]))
            case "div":
                operations.append(
                    translate_binary(
                        operands, loc, "div",
                        [OperationType.DIV_I, OperationType.DIV_R]))
            case "halt":
                if len(operands) != 0:
                    no_overload("halt", operands, loc)
                operations.append(Operation(OperationType.HALT, loc, []))

    return operations


REGISTERS = {
    "ip": 0,
    "sp": 1,
    "bp": 2,
    "ac": 3,
    "r1": 4,
    "r2": 5,
    "r3": 6,
    "r4": 7,
    "r5": 8,
    "r6": 9,
    "r7": 10,
    "r8": 11
}


def get_register(key: str, loc: Location) -> int:
    if key not in REGISTERS:
        report_error(f"unknown register {key}", loc)
        report_note(f"{', '.join(REGISTERS.keys())}", loc)
        exit(1)
    return REGISTERS[key]


def pass3(operations: List[Operation]) -> bytearray:
    """Compile the operations into bytecode"""
    bytecode = bytearray()

    def word_split(w) -> Tuple[int, int]:
        return [(w >> 8), (w & 0xFF)]

    for operation in operations:
        loc: Location = operation.loc
        operands = operation.val

        if operation.typ == OperationType.DIRECTIVE:
            if operation.val[0] == DirectiveType.DW:
                bytecode.extend(word_split(int(operands[1].val)))
            continue  # Directive not present in bytecode

        bytecode.append(operation.typ)

        match operation.typ:
            case OperationType.NOP:
                pass  # Nothing
            case OperationType.MOV_R_I:
                dest = get_register(operands[0].val, loc)
                bytecode.extend([dest] + word_split(int(operands[1].val)))
            case OperationType.MOV_R_R:
                dest = get_register(operands[0].val, loc)
                bytecode.extend([dest, get_register(operands[0].val, loc)])
            case OperationType.MOV_R_IM:
                dest = get_register(operands[0].val, loc)
                bytecode.extend([dest] + word_split(int(operands[1].val.val)))
            case OperationType.MOV_R_RM:
                dest = get_register(operands[0].val, loc)
                bytecode.extend([dest, get_register(operands[1].val.val, loc)])
            case OperationType.MOV_IM_I:
                dest = word_split(int(operands[0].val.val))
                bytecode.extend(dest + word_split(int(operands[1].val)))
            case OperationType.MOV_IM_R:
                dest = word_split(int(operands[0].val.val))
                bytecode.extend(dest + [get_register(operands[1].val, loc)])
            case OperationType.MOV_IM_IM:
                dest = word_split(int(operands[0].val.val))
                address = word_split(int(operands[1].val.val))
                bytecode.extend(dest + address)
            case OperationType.MOV_IM_RM:
                dest = word_split(int(operands[0].val.val))
                address = get_register(operands[1].val.val, loc)
                bytecode.extend(dest + [address])
            case OperationType.MOV_RM_I:
                dest = get_register(operands[0].val.val, loc)
                bytecode.extend([dest] + word_split(int(operands[1].val)))
            case OperationType.MOV_RM_R:
                dest = get_register(operands[0].val.val, loc)
                bytecode.extend([dest, get_register(operands[1].val, loc)])
            case OperationType.MOV_RM_IM:
                dest = get_register(operands[0].val.val, loc)
                address = word_split(int(operands[1].val.val))
                bytecode.extend([dest] + address)
            case OperationType.MOV_RM_RM:
                dest = get_register(operands[0].val.val, loc)
                address = get_register(operands[1].val.val, loc)
                bytecode.extend([dest, address])
            case OperationType.PUSH_I:
                bytecode.extend(word_split(int(operands[0].val)))
            case OperationType.PUSH_R:
                bytecode.extend([get_register(operands[0].val, loc)])
            case OperationType.POP:
                bytecode.extend([get_register(operands[0].val, loc)])
            case (OperationType.ADD_I | OperationType.SUB_I
                  | OperationType.MUL_I | OperationType.DIV_I):
                dest = get_register(operands[0].val, loc)
                src1 = get_register(operands[1].val, loc)
                src2 = word_split(int(operands[2].val))
                bytecode.extend([dest, src1] + src2)
            case (OperationType.ADD_R | OperationType.SUB_R
                  | OperationType.MUL_R | OperationType.DIV_R):
                dest = get_register(operands[0].val, loc)
                src1 = get_register(operands[1].val, loc)
                src2 = get_register(operands[2].val, loc)
                bytecode.extend([dest, src1, src2])
            case OperationType.HALT:
                pass  # Nothing

    return bytecode


def main() -> None:
    parser = ap.ArgumentParser(description="Assemble ASM into VM bytecode")

    parser.add_argument("input", type=str, help="Path to input file")
    parser.add_argument("output", type=str, help="Path to output file")

    args = parser.parse_args()
    # args: List[str] = sys.argv

    path = args.input
    with open(path, 'r') as f:
        text = f.read()

    lines = enumerate_lines(text)
    lines = preprocess_lines(lines, path)
    tokens = tokenize_lines(lines, path)
    # print(*tokens, sep='\n')

    labels = pass1(tokens)
    # print(*[(key, value) for key, value in labels.items()], sep='\n')

    operations = pass2(tokens, labels)
    # print(*operations, sep='\n')

    bytecode = pass3(operations)
    # fmt = ' '.join(f'{byte:02x}' for byte in bytecode)
    # print(fmt)

    with open(args.output, "wb") as f:
        f.write(bytecode)


if __name__ == "__main__":
    main()

