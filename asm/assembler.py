#!/usr/bin/python3

import sys
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
        # return f"({self.typ}: {self.val})"


@unique
class DirectiveType(Enum):
    DW = auto()


DIRECTIVES = {item.name.lower(): item for item in DirectiveType}


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

    # Custom OperationType, not present withing byte-code
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
    if fragment in DIRECTIVES:
        return Token(TokenType.DIRECTIVE, loc, DIRECTIVES[fragment])
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


# def get_label_value(labels: Dict[str, int], key: str) -> int:
#     if key not in labels:
#         report_error(f"invalid label {key}")
#     return labels[key]


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
    report_note(f"got {', '.join([str(item) for item in operands])}", loc)
    exit(1)


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

        # Skip non-operations
        if operation.typ != TokenType.SYMBOL:
            continue

        match operation.val:
            case "nop":
                if len(operands) != 0:
                    no_overload(operation, operands, loc)
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
                    no_overload(operation, operands, loc)
                operations.append(Operation(OperationType.HALT, loc, []))

    return operations


def main() -> None:
    args: List[str] = sys.argv

    path = args[1]
    with open(path, 'r') as f:
        text = f.read()

    lines = enumerate_lines(text)
    lines = preprocess_lines(lines, path)
    tokens = tokenize_lines(lines, path)
    # print(*tokens, sep='\n')

    labels = pass1(tokens)
    # print(*[(key, value) for key, value in labels.items()], sep='\n')

    operations = pass2(tokens, labels)
    for op in operations:
        print(f"{op.typ.name}: {op.val}")


if __name__ == "__main__":
    main()

