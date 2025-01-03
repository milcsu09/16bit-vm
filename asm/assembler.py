#!/usr/bin/python3

import os
import re
import sys
import inspect
import warnings

from enum import IntEnum, auto, unique

warnings.simplefilter("ignore", category=DeprecationWarning)


@unique
class TokenType(IntEnum):
    DIRECTIVE = auto(0)
    IMMEDIATE = auto()
    IDENTIFIER = auto()
    STRING = auto()
    LABEL = auto()
    IADDRESS = auto()
    RADDRESS = auto()

    def __str__(self):
        return self.name.lower()

    def __repr__(self):
        return str(self)


class Token:
    def __init__(self, loc, typ, val):
        self.loc = loc
        self.typ = typ
        self.val = val

    def __str__(self):
        return f"{self.typ} {repr(self.val)}"

    def __repr__(self):
        return str(self)


@unique
class DirectiveType(IntEnum):
    WORD = auto(0)
    ATTACH = auto()
    DEFINE = auto()
    RESERVE = auto()

    def __str__(self):
        return self.name.lower()

    def __repr__(self):
        return str(self)


DIRECTIVES = {str(item): item for item in DirectiveType}


@unique
class OperationType(IntEnum):
    NONE = 0
    MOV_R_I = auto()
    MOV_R_R = auto()
    MOV_R_IM = auto()
    MOV_R_RM = auto()
    MOV_IM_I = auto()
    MOV_IM_R = auto()
    MOV_IM_IM = auto()
    MOV_IM_RM = auto()
    MOV_RM_I = auto()
    MOV_RM_R = auto()
    MOV_RM_IM = auto()
    MOV_RM_RM = auto()
    PUSH_I = auto()
    PUSH_R = auto()
    POP = auto()
    PUSHA = auto()
    POPA = auto()
    ADD_I = auto()
    ADD_R = auto()
    SUB_I = auto()
    SUB_R = auto()
    MUL_I = auto()
    MUL_R = auto()
    DIV_I = auto()
    DIV_R = auto()
    CMP_I = auto()
    CMP_R = auto()
    JMP_I = auto()
    JMP_R = auto()
    JEQ_I = auto()
    JEQ_R = auto()
    JNE_I = auto()
    JNE_R = auto()
    JLT_I = auto()
    JLT_R = auto()
    JGT_I = auto()
    JGT_R = auto()
    JLE_I = auto()
    JLE_R = auto()
    JGE_I = auto()
    JGE_R = auto()
    CALL_I = auto()
    CALL_R = auto()
    RET = auto()
    HALT = auto()

    DIRECTIVE = auto()

    def __str__(self):
        return self.name.lower()

    def __repr__(self):
        return str(self)


class Operation:
    def __init__(self, loc, typ, val, full=False):
        self.loc = loc
        self.typ = typ
        self.val = val
        self.full = full

    def __str__(self):
        return f"{self.typ} {repr(self.val)}"

    def __repr__(self):
        return str(self)


def report_message(typ, msg, loc):
    if loc:
        context, line = loc
        sys.stderr.write(f"{context} [{line}]: ")
    sys.stderr.write(f"{typ}: {msg}\n")


def report_error(msg, loc):
    report_message("ERROR", msg, loc)


def report_warning(msg, loc):
    report_message("WARNING", msg, loc)


def report_note(msg, loc):
    report_message("NOTE", msg, loc)


def report_todo(msg):
    report_message("TODO", msg, None)


def report_invalid_fragment(fragment, loc):
    report_error(f"invalid fragment `{fragment}`", loc)


def report_redefinition(name, loc):
    report_error(f"redefinition of `{name}`", loc)


def report_undefined(name, loc):
    report_error(f"undefined `{name}`", loc)


def report_no_overload(operation, operands, loc):
    operands = ', '.join(str(x) for x in operands) or "nothing"
    report_error(f"no matching overload for {operation} with {operands}", loc)


def lexer_tokenize(string, context):
    tokens = []

    for number, line in enumerate(string.splitlines(), 1):
        line = line.split("//")[0].strip()
        if not line:
            continue

        tokens.append([
            lexer_tokenize_fragment(fragment, (context, number))
            for fragment in lexer_split(line)
        ])

    return tokens


def lexer_split(string):
    # Pattern to split a string at every whitespace, but preserve spaces inside
    # double or single quotes. Also preserves the quotes. Able to escape quotes
    # with `\`.
    pattern = '\'(?:\\\'|[^\'])*\'|\"(?:\\\"|[^\"])*\"|\S+'
    return re.findall(pattern, string)


def lexer_valid_identifier(string):
    return bool(re.match('^[A-Za-z_][A-Za-z0-9_]*$', string))

def lexer_encode_string(string, loc):
    try:
        return string.encode('utf-8').decode('unicode_escape')
    except UnicodeDecodeError:
        report_warning(f"invalid escape sequence", loc)
        report_note(f"will default to `{string}`", loc)
        return string


def lexer_tokenize_fragment(fragment, loc):
    if fragment in DIRECTIVES:
        return Token(loc, TokenType.DIRECTIVE, DIRECTIVES[fragment])

    elif fragment[0].isdigit():
        try:
            return Token(loc, TokenType.IMMEDIATE, int(fragment, 0))
        except:
            pass

    elif lexer_valid_identifier(fragment):
        return Token(loc, TokenType.IDENTIFIER, fragment)

    elif fragment[0] == fragment[-1] == "'" and len(fragment) > 1:
        child = lexer_encode_string(fragment[1:-1], loc)
        if len(child) == 1:
            return Token(loc, TokenType.IMMEDIATE, ord(child))

    elif fragment[0] == fragment[-1] == "\"" and len(fragment) > 1:
        child = lexer_encode_string(fragment[1:-1], loc)
        return Token(loc, TokenType.STRING, child)

    elif fragment[0] == "." and lexer_valid_identifier(fragment[1:]):
        return Token(loc, TokenType.LABEL, fragment[1:])

    elif fragment[0] == "*" and len(fragment) > 1:
        child = lexer_tokenize_fragment(fragment[1:], loc)
        if child.typ in (TokenType.IMMEDIATE, TokenType.LABEL):
            return Token(loc, TokenType.IADDRESS, child.val)
        if child.typ in (TokenType.IDENTIFIER, ):
            return Token(loc, TokenType.RADDRESS, child.val)

    report_invalid_fragment(fragment, loc)
    exit(1)


def transformer_expand_token(token, full):
    half = not full
    match token.typ:
        case TokenType.DIRECTIVE:
            # Directive won't expand.
            return 0
        case TokenType.IMMEDIATE:
            # Either 8- or 16-bit.
            return 2 - half
        case TokenType.IDENTIFIER:
            # After transformation, identifier will only hold registers.
            # Register expands to 1 byte.
            return 1
        case TokenType.STRING:
            # String will either be split into 8- or 16-bit values.
            return len(token.val) * (2 - half)
        case TokenType.LABEL:
            # TODO: figure out the correct way.
            return 2 - half
        case TokenType.IADDRESS:
            # IADDRESS is always 16-bit
            return 2
        case TokenType.RADDRESS:
            # RADDRESS is always 8-bit
            return 1


def transformer_expand_line(line, half):
    return sum(transformer_expand_token(token, half) for token in line)


def transformer_process_line(line, full):
    half = not full
    operation, *operands = line
    loc = operation.loc

    if len(operands) >= 1 and (operands[0].typ == TokenType.DIRECTIVE and
                               operands[0].val == DirectiveType.WORD):
        operands = operands[1:]
        return transformer_process_line([operation, *operands], True)

    # Since define can have any amount of arguments, it's checked here.
    # No need for return since define is passed on like any other operation.
    elif operation.val == DirectiveType.DEFINE:
        expected = [TokenType.IMMEDIATE, TokenType.STRING, TokenType.LABEL]
        if not all(got.typ in expected for got in operands):
            report_no_overload(operation, operands, loc)
            exit(1)

    # `Reserve` is special, since each word / byte isn't defined at compile
    # time, but given as an integer to know how many of them there are.
    elif operation.val == DirectiveType.RESERVE:
        if operation_match_arguments([TokenType.IMMEDIATE], operands):
            return full, operands[0].val * (2 - half), line
        report_no_overload(operation, operands, loc)
        exit(1)

    return full, transformer_expand_line(line, full), line


def transformer_process_lines(lines):
    result = []
    attach = []

    for line in lines:
        operation, *operands = line
        loc = operation.loc

        # `Attach` has to be handled here, since it will modify multiple lines
        # instead of a single line.
        if operation.val == DirectiveType.ATTACH:
            if operation_match_arguments([TokenType.STRING], operands):
                try:
                    file = operands[0].val
                    attach.extend(transformer_process_file(file, loc))
                except RecursionError:
                    report_error(f"`{file}` depends on itself", loc)
                    exit(1)
                continue
            report_no_overload(operation, operands, loc)
            exit(1)

        result.append(transformer_process_line(line, False))

    return result + attach


def transformer_process_file(file, loc=None):
    try:
        with open(file, "r") as f:
            content = f.read()
    except FileNotFoundError as e:
        report_error(str(e), loc)
        exit(1)

    lines = lexer_tokenize(content, file)
    lines = transformer_process_lines(lines)
    return lines


def transformer_parse_labels(lines):
    labels = {}
    address = 0

    for _, size, data in lines:
        operation, *operands = data
        loc = operation.loc

        if operation.typ == TokenType.LABEL:
            if operation_match_arguments([], operands):
                labels[operation.val] = address
                continue
            report_no_overload(operation, operands, loc)
            exit(1)

        address += size

    return labels


def operation_match_argument(expected, got):
    # When `expected` is None, it acts as an `Any` type.
    if expected == None:
        return True
    return expected == got.typ


def operation_match_arguments(expected, got):
    if len(expected) != len(got):
        return False
    return all(operation_match_argument(a, b) for a, b in zip(expected, got))


# This huge dictionary is a metadata for the operations. It stores each
# overload based on the given arguments.
OVERLOADS = {
    "none": OperationType.NONE,

    "mov": [
        ([TokenType.IDENTIFIER, TokenType.IMMEDIATE], OperationType.MOV_R_I),
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER], OperationType.MOV_R_R),
        ([TokenType.IDENTIFIER, TokenType.IADDRESS], OperationType.MOV_R_IM),
        ([TokenType.IDENTIFIER, TokenType.RADDRESS], OperationType.MOV_R_RM),

        ([TokenType.IADDRESS, TokenType.IMMEDIATE], OperationType.MOV_IM_I),
        ([TokenType.IADDRESS, TokenType.IDENTIFIER], OperationType.MOV_IM_R),
        ([TokenType.IADDRESS, TokenType.IADDRESS], OperationType.MOV_IM_IM),
        ([TokenType.IADDRESS, TokenType.RADDRESS], OperationType.MOV_IM_RM),

        ([TokenType.RADDRESS, TokenType.IMMEDIATE], OperationType.MOV_RM_I),
        ([TokenType.RADDRESS, TokenType.IDENTIFIER], OperationType.MOV_RM_R),
        ([TokenType.RADDRESS, TokenType.IADDRESS], OperationType.MOV_RM_IM),
        ([TokenType.RADDRESS, TokenType.RADDRESS], OperationType.MOV_RM_RM),
    ],

    "push": [
        ([TokenType.IMMEDIATE], OperationType.PUSH_I),
        ([TokenType.IDENTIFIER], OperationType.PUSH_R),
    ],

    "pop": [
        ([TokenType.IDENTIFIER], OperationType.POP),
    ],

    "pusha": OperationType.PUSHA,
    "popa": OperationType.POPA,

    "add": [
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER, TokenType.IMMEDIATE],
         OperationType.ADD_I),
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER, TokenType.IDENTIFIER],
         OperationType.ADD_R),
    ],

    "sub": [
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER, TokenType.IMMEDIATE],
         OperationType.SUB_I),
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER, TokenType.IDENTIFIER],
         OperationType.SUB_R),
    ],

    "mul": [
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER, TokenType.IMMEDIATE],
         OperationType.MUL_I),
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER, TokenType.IDENTIFIER],
         OperationType.MUL_R),
    ],

    "div": [
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER, TokenType.IMMEDIATE],
         OperationType.DIV_I),
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER, TokenType.IDENTIFIER],
         OperationType.DIV_R),
    ],

    "cmp": [
        ([TokenType.IDENTIFIER, TokenType.IMMEDIATE], OperationType.CMP_I),
        ([TokenType.IDENTIFIER, TokenType.IDENTIFIER], OperationType.CMP_R),
    ],

    "jmp": [
        ([TokenType.IMMEDIATE], OperationType.JMP_I),
        ([TokenType.IDENTIFIER], OperationType.JMP_R),
    ],

    "jeq": [
        ([TokenType.IMMEDIATE], OperationType.JEQ_I),
        ([TokenType.IDENTIFIER], OperationType.JEQ_R),
    ],

    "jne": [
        ([TokenType.IMMEDIATE], OperationType.JNE_I),
        ([TokenType.IDENTIFIER], OperationType.JNE_R),
    ],

    "jlt": [
        ([TokenType.IMMEDIATE], OperationType.JLT_I),
        ([TokenType.IDENTIFIER], OperationType.JLT_R),
    ],

    "jgt": [
        ([TokenType.IMMEDIATE], OperationType.JGT_I),
        ([TokenType.IDENTIFIER], OperationType.JGT_R),
    ],

    "jle": [
        ([TokenType.IMMEDIATE], OperationType.JLE_I),
        ([TokenType.IDENTIFIER], OperationType.JLE_R),
    ],

    "jge": [
        ([TokenType.IMMEDIATE], OperationType.JGE_I),
        ([TokenType.IDENTIFIER], OperationType.JGE_R),
    ],

    "call": [
        ([TokenType.IMMEDIATE], OperationType.CALL_I),
        ([TokenType.IDENTIFIER], OperationType.CALL_R),
    ],

    "ret": OperationType.RET,
    "halt": OperationType.HALT,
}


def operation_find_overload(operation, operands, table):
    if operation.val not in table:
        report_no_overload(operation, operands, operation.loc)
        exit(1)

    overloads = table[operation.val]

    if isinstance(overloads, IntEnum):
        if operands == []:
            return overloads
    else:
        for overload in overloads:
            expected, result = overload
            if operation_match_arguments(expected, operands):
                return result

    report_no_overload(operation, operands, operation.loc)
    exit(1)


def transformer_process_operands(operands, labels):
    def get_label(operand):
        if operand.val not in labels:
            report_undefined(operand.val, operand.loc)
            exit(1)
        return labels[operand.val]

    result = operands.copy()
    for i, operand in enumerate(result):
        if operand.typ == TokenType.LABEL:
            child = get_label(operand)
            result[i] = Token(operand.loc, TokenType.IMMEDIATE, child)
        if operand.typ == TokenType.IADDRESS:
            if isinstance(operand.val, str):
                child = get_label(operand)
                result[i].val = child

    return result


def transformer_parse_operations(lines, labels):
    result = []

    for full, size, data in lines:
        operation, *operands = data
        loc = operation.loc
    
        if operation.typ == TokenType.LABEL:
            continue

        operands = transformer_process_operands(operands, labels)

        # Directives are passed on with a wrapper.
        if operation.typ == TokenType.DIRECTIVE:
            operands = [operation.val] + operands
            result.append(Operation(loc, OperationType.DIRECTIVE, operands,
                                    full))
            continue

        # Find the overload for the operation.
        overload = operation_find_overload(operation, operands, OVERLOADS)
        result.append(Operation(loc, overload, operands, full))

    return result


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


def builder_get_register(key, loc):
    if key not in REGISTERS:
        report_undefined(key, loc)
        exit(1)
    return REGISTERS[key]


def builder_fit_limit(value, limit, loc):
    if value >= limit:
        report_warning(f"immediate `{value}` cannot fit target type", loc)
        value %= limit
        report_note(f"will default to `{value}`", loc)
    return value


def builder_get_width(value, full, loc):
    if not full:
        return [builder_fit_limit(value, 0x100, loc)]
    value = builder_fit_limit(value, 0x10000, loc)
    return [(value & 0xFF), (value >> 8)]


def builder_build_operand(operand, full, loc):
    result = bytearray()

    if operand.typ == TokenType.IMMEDIATE:
        result.extend(builder_get_width(operand.val, full, loc))
    if operand.typ == TokenType.IDENTIFIER:
        result.append(builder_get_register(operand.val, loc))
    if operand.typ == TokenType.STRING:
        for char in operand.val:
            result.extend(builder_get_width(ord(char), full, loc))
    if operand.typ == TokenType.IADDRESS:
        result.extend(builder_get_width(operand.val, True, loc))
    if operand.typ == TokenType.RADDRESS:
        result.append(builder_get_register(operand.val, loc))

    return result


def builder_build(operations):
    result = bytearray()

    for operation in operations:
        loc = operation.loc
        full = operation.full
        operation, operands = operation.typ, operation.val

        if operation == OperationType.DIRECTIVE:
            operation, *operands = operands

            if operation == DirectiveType.DEFINE:
                for operand in operands:
                    result.extend(builder_build_operand(operand, full, loc))

            if operation == DirectiveType.RESERVE:
                for _ in range(operands[0].val):
                    result.extend(builder_get_width(0, full, loc))

            continue

        if full:
            result.append(operation | 0x80)
        else:
            result.append(operation)

        for operand in operands:
            result.extend(builder_build_operand(operand, full, loc))

    return result


def main():
    input_path = sys.argv[1]
    lines = transformer_process_file(input_path)
    labels = transformer_parse_labels(lines)

    operations = transformer_parse_operations(lines, labels)
    result = builder_build(operations)

    output_path = os.path.splitext(input_path)[0]
    with open(output_path, "wb") as f:
        f.write(result)

    if len(labels) != 0:
        print(f"{len(labels)} labels")
        pad = len(max(labels.keys(), key=len))
        previous = 0
        for key, value in labels.items():
            diff = value - previous
            print(f"    {key:{pad}} {value:04X} ({value}) +{abs(diff)}")
            previous = value

    print(f"Success. {len(result)} bytes, '{output_path}'")

if __name__ == "__main__":
    main()

