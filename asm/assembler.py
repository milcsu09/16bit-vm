#!/usr/bin/python3

import time
import os, sys
from enum import IntEnum, auto, unique


def report_message(typ, msg, loc=None, fd=sys.stderr):
    fd.write(f"{loc[0]} [{loc[1]}]: " if loc else "")
    fd.write(f"{typ}: {msg}\n")


def report_error(*args, **kwargs):
    report_message("ERROR", *args, **kwargs)


def report_warning(*args, **kwargs):
    report_message("WARNING", *args, **kwargs)


def report_note(*args, **kwargs):
    report_message("NOTE", *args, **kwargs)


@unique
class TokenType(IntEnum):
    DIRECTIVE = 0

    SYMBOL = auto()
    NUMBER = auto()
    STRING = auto()

    EQUALS = auto()
    # Colon implicitly places an EOL token after itself.
    COLON = auto()

    PLUS = auto()
    MINUS = auto()
    STAR = auto()
    SLASH = auto()
    SHL = auto()
    SHR = auto()

    LPAREN = auto()
    RPAREN = auto()
    LBRACE = auto()
    RBRACE = auto()
    LBRACKET = auto()
    RBRACKET = auto()

    EOL = auto()
    EOF = auto()

    # Used after tokenization later
    IMEMORY = auto()
    RMEMORY = auto()

    def __str__(self):
        return self.name.upper()

    def __repr__(self):
        return str(self)


class Token:
    def __init__(self, loc, typ, val=None):
        self.loc = loc
        self.typ = typ
        self.val = val

    def to(self, typ, val=None):
        return Token(self.loc, typ, self.val if val is None else val)

    def __str__(self):
        # postfix = f":{repr(self.val)}" if self.val is not None else ""
        # return str(self.typ) + postfix
        return repr(self.val)

    def __repr__(self):
        return str(self)


TOKEN_SINGLE = {
    "=": TokenType.EQUALS,
    ":": TokenType.COLON,
    "+": TokenType.PLUS,
    "-": TokenType.MINUS,
    "*": TokenType.STAR,
    "/": TokenType.SLASH,
    "(": TokenType.LPAREN,
    ")": TokenType.RPAREN,
    "{": TokenType.LBRACE,
    "}": TokenType.RBRACE,
    "[": TokenType.LBRACKET,
    "]": TokenType.RBRACKET,
    "\\": TokenType.EOL,
    "\n": TokenType.EOL,
}


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


# Has to align with the VM's instruction set!
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
    MOVB_R_I = auto()
    MOVB_R_R = auto()
    MOVB_R_IM = auto()
    MOVB_R_RM = auto()
    MOVB_IM_I = auto()
    MOVB_IM_R = auto()
    MOVB_IM_IM = auto()
    MOVB_IM_RM = auto()
    MOVB_RM_I = auto()
    MOVB_RM_R = auto()
    MOVB_RM_IM = auto()
    MOVB_RM_RM = auto()
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
    AND_I = auto()
    AND_R = auto()
    OR_I = auto()
    OR_R = auto()
    XOR_I = auto()
    XOR_R = auto()
    NOT = auto()
    SHL_I = auto()
    SHL_R = auto()
    SHR_I = auto()
    SHR_R = auto()
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
    PRINT_I = auto()
    PRINT_R = auto()

    DIRECTIVE = auto()

    def __str__(self):
        return self.name.upper()

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


# Operation overloads.
OPERATIONS = {
    "none": OperationType.NONE,

    "mov": [
        ([TokenType.SYMBOL, TokenType.NUMBER], OperationType.MOV_R_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL], OperationType.MOV_R_R),
        ([TokenType.SYMBOL, TokenType.IMEMORY], OperationType.MOV_R_IM),
        ([TokenType.SYMBOL, TokenType.RMEMORY], OperationType.MOV_R_RM),

        ([TokenType.IMEMORY, TokenType.NUMBER], OperationType.MOV_IM_I),
        ([TokenType.IMEMORY, TokenType.SYMBOL], OperationType.MOV_IM_R),
        ([TokenType.IMEMORY, TokenType.IMEMORY], OperationType.MOV_IM_IM),
        ([TokenType.IMEMORY, TokenType.RMEMORY], OperationType.MOV_IM_RM),

        ([TokenType.RMEMORY, TokenType.NUMBER], OperationType.MOV_RM_I),
        ([TokenType.RMEMORY, TokenType.SYMBOL], OperationType.MOV_RM_R),
        ([TokenType.RMEMORY, TokenType.IMEMORY], OperationType.MOV_RM_IM),
        ([TokenType.RMEMORY, TokenType.RMEMORY], OperationType.MOV_RM_RM),
    ],

    "movb": [
        ([TokenType.SYMBOL, TokenType.NUMBER], OperationType.MOVB_R_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL], OperationType.MOVB_R_R),
        ([TokenType.SYMBOL, TokenType.IMEMORY], OperationType.MOVB_R_IM),
        ([TokenType.SYMBOL, TokenType.RMEMORY], OperationType.MOVB_R_RM),

        ([TokenType.IMEMORY, TokenType.NUMBER], OperationType.MOVB_IM_I),
        ([TokenType.IMEMORY, TokenType.SYMBOL], OperationType.MOVB_IM_R),
        ([TokenType.IMEMORY, TokenType.IMEMORY], OperationType.MOVB_IM_IM),
        ([TokenType.IMEMORY, TokenType.RMEMORY], OperationType.MOVB_IM_RM),

        ([TokenType.RMEMORY, TokenType.NUMBER], OperationType.MOVB_RM_I),
        ([TokenType.RMEMORY, TokenType.SYMBOL], OperationType.MOVB_RM_R),
        ([TokenType.RMEMORY, TokenType.IMEMORY], OperationType.MOVB_RM_IM),
        ([TokenType.RMEMORY, TokenType.RMEMORY], OperationType.MOVB_RM_RM),
    ],

    "push": [
        ([TokenType.NUMBER], OperationType.PUSH_I),
        ([TokenType.SYMBOL], OperationType.PUSH_R),
    ],

    "pop": [
        ([TokenType.SYMBOL], OperationType.POP),
    ],

    "pusha": OperationType.PUSHA,
    "popa": OperationType.POPA,

    "add": [
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.NUMBER],
         OperationType.ADD_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL],
         OperationType.ADD_R),
    ],

    "sub": [
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.NUMBER],
         OperationType.SUB_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL],
         OperationType.SUB_R),
    ],

    "mul": [
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.NUMBER],
         OperationType.MUL_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL],
         OperationType.MUL_R),
    ],

    "div": [
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.NUMBER],
         OperationType.DIV_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL],
         OperationType.DIV_R),
    ],

    "and": [
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.NUMBER],
         OperationType.AND_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL],
         OperationType.AND_R),
    ],

    "or": [
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.NUMBER],
         OperationType.OR_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL],
         OperationType.OR_R),
    ],

    "xor": [
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.NUMBER],
         OperationType.XOR_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL],
         OperationType.XOR_R),
    ],

    "not": [
        ([TokenType.SYMBOL, TokenType.SYMBOL], OperationType.NOT),
    ],

    "shl": [
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.NUMBER],
         OperationType.SHL_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL],
         OperationType.SHL_R),
    ],

    "shr": [
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.NUMBER],
         OperationType.SHR_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL, TokenType.SYMBOL],
         OperationType.SHR_R),
    ],

    "cmp": [
        ([TokenType.SYMBOL, TokenType.NUMBER], OperationType.CMP_I),
        ([TokenType.SYMBOL, TokenType.SYMBOL], OperationType.CMP_R),
    ],

    "jmp": [
        ([TokenType.NUMBER], OperationType.JMP_I),
        ([TokenType.SYMBOL], OperationType.JMP_R),
    ],

    "jeq": [
        ([TokenType.NUMBER], OperationType.JEQ_I),
        ([TokenType.SYMBOL], OperationType.JEQ_R),
    ],

    "jne": [
        ([TokenType.NUMBER], OperationType.JNE_I),
        ([TokenType.SYMBOL], OperationType.JNE_R),
    ],

    "jlt": [
        ([TokenType.NUMBER], OperationType.JLT_I),
        ([TokenType.SYMBOL], OperationType.JLT_R),
    ],

    "jgt": [
        ([TokenType.NUMBER], OperationType.JGT_I),
        ([TokenType.SYMBOL], OperationType.JGT_R),
    ],

    "jle": [
        ([TokenType.NUMBER], OperationType.JLE_I),
        ([TokenType.SYMBOL], OperationType.JLE_R),
    ],

    "jge": [
        ([TokenType.NUMBER], OperationType.JGE_I),
        ([TokenType.SYMBOL], OperationType.JGE_R),
    ],

    "call": [
        ([TokenType.NUMBER], OperationType.CALL_I),
        ([TokenType.SYMBOL], OperationType.CALL_R),
    ],

    "ret": OperationType.RET,
    "halt": OperationType.HALT,

    "print": [
        ([TokenType.NUMBER], OperationType.PRINT_I),
        ([TokenType.SYMBOL], OperationType.PRINT_R),
    ],
}


# These operations will always be implicitly full to avoid problems.
# ALWAYS_FULL = [
#     "jmp",
#     "jeq",
#     "jne",
#     "jlt",
#     "jgt",
#     "jle",
#     "jge",
#     "call",
# ]

ALWAYS_HALF = [
    "movb",
]


@unique
class DirectiveType(IntEnum):
    # W = 0
    INCLUDE = auto()
    ATTACH = auto()
    DEF = auto()
    RES = auto()
    DEFB = auto()
    RESB = auto()

    def __str__(self):
        return self.name.lower()

    def __repr__(self):
        return str(self)


DIRECTIVES = {str(item): item for item in DirectiveType}


def lexer_find(content, condition, start):
    while start < len(content) and not condition(content[start]):
        start = start + 1
    return start


def lexer_number(content, loc):
    try:
      if '.' in content:
          float_value = float(content)
          value = int(float_value * 256)
      else:
          value = int(content, 0)
    except ValueError:
        report_error(f"invalid `{content}`", loc)
        exit(1)
    return Token(loc, TokenType.NUMBER, value)


def lexer_encode_string(string, loc):
    try:
        return string.encode('utf-8').decode('unicode_escape')
    except UnicodeDecodeError:
        report_error(f"invalid escape sequence", loc)
        exit(1)


def lexer_tokenize_file(path, loc=None):
    try:
        with open(path, "r") as file:
            content = file.read()
            length = len(content)
    except:
        report_error(f"unable to read `{path}`", loc)
        exit(1)

    index, line = 0, 1
    while index < length:
        loc, begin = (path, line), index

        if content[index].isalpha() or content[index] == "_":
            condition = lambda x: not (x.isalnum() or x == "_")
            index = lexer_find(content, condition, index) - 1
            value = content[begin:index + 1]
            if value in DIRECTIVES:
                yield Token(loc, TokenType.DIRECTIVE, DIRECTIVES[value])
            else:
                yield Token(loc, TokenType.SYMBOL, value)

        elif content[index].isdigit() or content[index] == ".":
            condition = lambda x: not x.isalnum() and x != "."
            index = lexer_find(content, condition, index) - 1
            yield lexer_number(content[begin:index + 1], loc)

        elif content[index] == "\"":
            condition = lambda x: x == "\""
            while index := lexer_find(content, condition, index + 1):
                if index >= length:
                    report_error("string-literal not terminated", loc)
                    exit(1)
                # Allow for escaped quotes
                elif ((value := content[begin + 1:index]) or "\0")[-1] != "\\":
                    break

            value = lexer_encode_string(value, loc)
            yield Token(loc, TokenType.STRING, value)

        elif content[index] == "'":
            index = index + 1 + (content[index + 1] == "\\")
            value = lexer_encode_string(content[begin + 1:index + 1], loc)
            if len(value) != 1:
                report_error(f"`{value}` too long", loc)
                exit(1)
            yield Token(loc, TokenType.NUMBER, ord(value))

        elif content[index:index + 2] == "<<":
            yield Token(loc, TokenType.SHL, "<<")
            index = index + 1

        elif content[index:index + 2] == ">>":
            yield Token(loc, TokenType.SHR, ">>")
            index = index + 1

        elif content[index] in TOKEN_SINGLE:
            yield Token(loc, TOKEN_SINGLE[content[index]], content[index])
            if content[index] in [":",]:
                yield Token(loc, TokenType.EOL, content[index])

        elif content[index:index + 2] == "#[": # ]
            stack = 1
            while index < length and stack != 0:
                index = index + 1
                if content[index:index + 2] == "#[": # ]
                    stack = stack + 1
                    index = index + 1
                if content[index:index + 2] == "]#":
                    stack = stack - 1
                    index = index + 1

            if index == length or stack != 0:
                report_error(f"unbalanced comment blocks", loc)
                exit(1)

        elif content[index] == "#":
            index = lexer_find(content, lambda x: x == "\n", index + 1) - 1

        elif not content[index].isspace():
            report_error(f"undefined `{content[index]}`", loc)
            exit(1)

        # Skip whitespaces, but not newlines!
        index = lexer_find(content, lambda x: x not in " \t\r", index + 1)
        line = line + content[begin:index].count("\n")

    yield Token((path, line - 1), TokenType.EOF)


def assert_token(token, *types):
    if token.typ not in (types or [token.typ]):
        report_error(f"expected {', '.join(str(typ) for typ in types)}",
                     token.loc)
        report_note(f"got {str(token.typ)}", token.loc)
        exit(1)


def assert_token_not(token, *types):
    if token.typ in types:
        report_error(f"unexpected {str(token.typ)}", token.loc)
        exit(1)


VALID_CONST = [TokenType.NUMBER, TokenType.SYMBOL]
VALID_EXPRESSION = [
    TokenType.NUMBER,
    TokenType.SYMBOL,
    TokenType.PLUS,
    TokenType.MINUS,
    TokenType.STAR,
    TokenType.SLASH,
    TokenType.SHL,
    TokenType.SHR,
    TokenType.LPAREN,
    TokenType.RPAREN,
]

# Handle constants, macros and the "w" directive.
def pass1(tokens, consts=None, macros=None):
    if consts is None:
        consts = {}

    if macros is None:
        macros = {}

    result = []
    attach = []

    full = True
    iterator = iter(tokens)

    def optional(*types):
        nonlocal current
        while current.typ != TokenType.EOF and current.typ in types:
            current = next(iterator, None)

    def expression():
        nonlocal current

        if current.typ != TokenType.LBRACKET:
            return current

        current = next(iterator, None)

        expr = ""
        body = []
        while current.typ not in [TokenType.RBRACKET, TokenType.EOL,
                                  TokenType.EOF]:
            assert_token(current, *VALID_EXPRESSION)
            body.append(current)
            current = next(iterator, None)

        body = body + [Token(initial.loc, TokenType.EOL)]
        body = pass1(body, {} | consts, {} | macros)[:-1]
        expr = " ".join(str(token.val) for _, token in body)

        try:
            x = int(eval(expr, {"__builtins__": None}, {}))
        except:
            report_error(f"invalid expression `{expr}`", initial.loc)
            exit(1)

        return Token(initial.loc, TokenType.NUMBER, x)

    current = next(iterator, None)
    while current != None:
        initial = current
        current = next(iterator, None)

        if initial.typ == TokenType.EOL:
            full = True

        if current and current.typ == TokenType.LBRACKET:
            result.append((full, initial))
            result.append((full, expression()))
            current = next(iterator, None)
            continue

        # Where directive is the initial token
        if initial.typ == TokenType.DIRECTIVE:
            # TODO: Remove repetition
            if initial.val == DirectiveType.INCLUDE:
                tokens = lexer_tokenize_file(current.val, current.loc)
                result.extend(pass1(tokens, consts, macros))
                current = next(iterator, current)
                continue

            if initial.val == DirectiveType.ATTACH:
                tokens = lexer_tokenize_file(current.val, current.loc)
                attach.extend(pass1(tokens, consts, macros))
                current = next(iterator, current)
                continue

        # Where is the current token
        # if current and current.typ == TokenType.DIRECTIVE:
        #     # Handle 16-bit operations using "w", short for "word"
        #     # print(initial, current)
        #     if current.val == DirectiveType.W:
        #         full = True
        #         current = next(iterator, current)
        #         result.append((full, initial))
        #         continue

        # Handle every symbol
        if initial.typ == TokenType.SYMBOL:
            # "Expand" consts
            if initial.val in consts:
                result.append((full, consts[initial.val]))
                continue

            # "Expand" macros
            if initial.val in macros:
                arguments = []
                while current.typ not in [TokenType.EOL, TokenType.EOF]:
                    assert_token(current, *VALID_CONST)
                    arguments.append(current)
                    current = next(iterator, None)

                # `+ [current]` to have an EOL or EOF token.
                arguments = arguments + [current]
                # `{} | x` to pass a copy of a dictionary
                expanded = pass1(arguments, {} | consts, {} | macros)
                arguments = [e[1] for e in expanded[:-1]]

                macro = macros[initial.val]
                expected, body = macro

                if len(expected) != len(arguments):
                    report_error("argument mismatch", initial.loc)
                    exit(1)

                local = dict(zip([x.val for x in expected], arguments))
                body = pass1(body, {} | consts | local, {} | macros)

                result.extend(body)
                continue

            # Only valid syntax is ```<symbol> = ...```
            if current.typ not in [TokenType.EQUALS]:
                result.append((full, initial))
                continue

            name = initial

            if current.typ == TokenType.EQUALS:
                current = next(iterator, None)

                optional(TokenType.EOL)

                # Handle macro assignment
                if current.typ in [TokenType.SYMBOL, TokenType.LBRACE]:
                    arguments = []
                    while current.typ in [TokenType.SYMBOL]:
                        arguments.append(current)
                        current = next(iterator, None)

                    optional(TokenType.EOL)

                    assert_token(current, TokenType.LBRACE)
                    current = next(iterator, None)

                    body = []
                    while current.typ not in [TokenType.RBRACE, TokenType.EOF]:
                        assert_token_not(current, TokenType.EQUALS)
                        body.append(current)
                        current = next(iterator, None)

                    assert_token(current, TokenType.RBRACE)
                    current = next(iterator, None)

                    macros[name.val] = (arguments, body)
                # Handle const assignment
                else:
                    consts[name.val] = expression()
                    current = next(iterator, None)
                continue
        else:
            result.append((full, initial))

    return result + attach


# Returns how much each operation will expand to during the generation of the
# bytecode.
def pass2_expand_token(token, full):
    match token.typ:
        case TokenType.DIRECTIVE:
            return 0
        case TokenType.SYMBOL:
            if token.val in REGISTERS | OPERATIONS:
                return 1
            return 2
        case TokenType.NUMBER:
            return 1 + full
        case TokenType.STRING:
            return len(token.val) * (1 + full)
        case TokenType.IMEMORY:
            return 2
        case TokenType.RMEMORY:
            if token.val in REGISTERS | OPERATIONS:
                return 1
            return 2
        case TokenType.COLON:
            return 0
        case TokenType.EOL:
            return 0
        case TokenType.EOF:
            return 0

    report_error(f"invalid syntax `{token}`", token.loc)
    exit(1)


def pass2_expand_line(line, full):
    return sum(pass2_expand_token(token, full) for token in line)


# Turn the tokens into intermediate lines.
# Handle more directives.
def pass2(trans):
    result = []

    iterator = iter(trans)

    current = next(iterator, None)
    while current != None:
        body = []
        initial = current
        full = initial[0]

        if initial[1].val in ALWAYS_HALF:
            full = 0

        # Handle directives
        if initial[1].typ == TokenType.DIRECTIVE:
            current = next(iterator, None)
            body = [initial[1]]

            # Handle directives
            if initial[1].val == DirectiveType.DEF:
                # Def can accept infinite amount of bytes!
                while current[1].typ not in [TokenType.EOL, TokenType.EOF]:
                    assert_token(current[1], *VALID_CONST, TokenType.STRING)
                    body.append(current[1])
                    current = next(iterator, None)

                result.append((1, pass2_expand_line(body, 1), body))
                continue

            if initial[1].val == DirectiveType.RES:
                assert_token(current[1], TokenType.NUMBER)
                amount = current[1].val
                body.append(current[1])

                result.append((1, amount * 2, body))

                current = next(iterator, None)
                continue

            if initial[1].val == DirectiveType.DEFB:
                # Def can accept infinite amount of bytes!
                while current[1].typ not in [TokenType.EOL, TokenType.EOF]:
                    assert_token(current[1], *VALID_CONST, TokenType.STRING)
                    body.append(current[1])
                    current = next(iterator, None)

                result.append((0, pass2_expand_line(body, 0), body))
                continue

            if initial[1].val == DirectiveType.RESB:
                assert_token(current[1], TokenType.NUMBER)
                amount = current[1].val
                body.append(current[1])

                result.append((0, amount * 1, body))

                current = next(iterator, None)
                continue

        # Handle normal operations
        while current[1].typ not in [TokenType.EOL, TokenType.EOF]:
            if current[1].typ == TokenType.LPAREN:
                current = next(iterator, None)
                assert_token(current[1], TokenType.SYMBOL, TokenType.NUMBER)

                if current[1].typ == TokenType.SYMBOL:
                    body.append(current[1].to(TokenType.RMEMORY))
                elif current[1].typ == TokenType.NUMBER:
                    body.append(current[1].to(TokenType.IMEMORY))

                current = next(iterator, None)
                assert_token(current[1], TokenType.RPAREN)
            else:
                body.append(current[1])

            current = next(iterator, None)

        # Now is the time to remove empty lines!
        if body != []:
            result.append((full, pass2_expand_line(body, full), body))
        current = next(iterator, None)

    # Each 'IR' has this format: (FULL, SIZE, TOKENS)
    # Full denotes if the operation is 8- or 16-bit
    # Size denotes the amount of bytes the operation will "expand" to.
    return result


# Calculate labels.
def pass3(ir):
    result = []
    labels = {}
    address = 0

    for full, size, tokens in ir:
        if len(tokens) == 2:
            if tokens[0].typ == TokenType.SYMBOL:
                if tokens[1].typ == TokenType.COLON:
                    labels[tokens[0].val] = address
                    continue
        address += size
        result.append((full, size, tokens))

    return result, labels


# Handle symbols.
def pass4(ir, labels):
    result = []

    for full, _, tokens in ir:
        copy = tokens.copy()

        for i, token in enumerate(tokens):
            # Handle symbols which are not defined. At this point, there's no
            # other way of having a defined symbol, then it either being a
            # register, operations or part of the labels dict.
            if token.typ == TokenType.SYMBOL:
                if token.val not in REGISTERS | OPERATIONS | labels:
                    report_error(f"undefined `{token.val}`", token.loc)
                    exit(1)

            # Substitute symbol tokens that are present in the labels dict,
            # with their address.
            if token.typ == TokenType.SYMBOL and token.val in labels:
                if tokens[0].val in ALWAYS_HALF and not full:
                    report_warning ("label referenced in half-sized operation",
                                    token.loc)
                copy[i] = token.to(TokenType.NUMBER, labels[token.val])
            elif token.typ == TokenType.RMEMORY and token.val not in REGISTERS:
                if token.val not in labels:
                    report_error(f"undefined `{token.val}`", token.loc)
                    exit(1)
                copy[i] = token.to(TokenType.IMEMORY, labels[token.val])

        # Don't append `size` back, we don't need it no more!
        result.append((full, copy))

    return result


def operation_match_argument(expected, got):
    # When `expected` is None, it acts as an `Any` type.
    if expected == None:
        return True
    return expected == got.typ


def operation_match_arguments(expected, got):
    if len(expected) != len(got):
        return False
    return all(operation_match_argument(a, b) for a, b in zip(expected, got))


def operation_find_overload(operation, operands, table):
    if operation.val not in table:
        report_error(f"invalid operation `{operation.val}`", operation.loc)
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

    report_error(f"invalid overload for `{operation.val}`", operation.loc)
    exit(1)


def parse_operations(ir):
    result = []

    for full, tokens in ir:
        operation, *operands = tokens
        loc = operation.loc

        if operation.val in ALWAYS_HALF:
            full = 0
        # full = full or operation.val in ALWAYS_FULL

        # Directives are just passed on. Their arguments have been checked in
        # previous passes.
        if operation.typ == TokenType.DIRECTIVE:
            operands = [operation.val] + operands
            result.append(Operation(loc, OperationType.DIRECTIVE, operands,
                                    full))
            continue

        overload = operation_find_overload(operation, operands, OPERATIONS)
        result.append(Operation(loc, overload, operands, full))

    return result


def build_fit_limit(value, limit, loc):
    if value > limit:
        t, value = value, value & limit
        report_warning(f"`{t}` wrapped to `{value}`", loc)
    return value


def build_get_width(value, full, loc):
    if not full:
        return [build_fit_limit(value, 0xff, loc)]
    value = build_fit_limit(value, 0xffff, loc)
    return [(value & 0xff), (value >> 8)]


def build_operand(operand, full, loc):
    result = bytearray()

    if operand.typ == TokenType.SYMBOL:
        result.append(REGISTERS[operand.val])
    elif operand.typ == TokenType.NUMBER:
        result.extend(build_get_width(operand.val, full, loc))
    elif operand.typ == TokenType.STRING:
        for char in operand.val:
            result.extend(build_get_width(ord(char), full, loc))
    elif operand.typ == TokenType.IMEMORY:
        result.extend(build_get_width(operand.val, True, loc))
    elif operand.typ == TokenType.RMEMORY:
        result.append(REGISTERS[operand.val])

    return result


def build(operations):
    result = bytearray()

    for operation in operations:
        loc = operation.loc
        full = operation.full
        operation, operands = operation.typ, operation.val

        # Build the directives first.
        if operation == OperationType.DIRECTIVE:
            operation, *operands = operands

            if operation == DirectiveType.DEF:
                for operand in operands:
                    result.extend(build_operand(operand, full, loc))

            elif operation == DirectiveType.RES:
                for _ in range(operands[0].val):
                    result.extend(build_get_width(0, full, loc))

            elif operation == DirectiveType.DEFB:
                for operand in operands:
                    result.extend(build_operand(operand, 0, loc))

            elif operation == DirectiveType.RESB:
                for _ in range(operands[0].val):
                    result.extend(build_get_width(0, 0, loc))


            continue

        # In the VM, each operations highest bit denotes the full flag.
        result.append(operation)

        for operand in operands:
            result.extend(build_operand(operand, full, loc))

    return result


def usage():
    print(f"Usage: {sys.argv[0]} [OPTIONS] <file>")
    print(f"    OPTIONS")
    print(f"        -d      Debug information about compilation process")
    print(f"        -x      Display compiled bytecode")


DEBUG_SEPARATOR = [
    # OperationType.JMP_I,
    # OperationType.JMP_R,
    # OperationType.JEQ_I,
    # OperationType.JEQ_R,
    # OperationType.JNE_I,
    # OperationType.JNE_R,
    # OperationType.JLT_I,
    # OperationType.JLT_R,
    # OperationType.JGT_I,
    # OperationType.JGT_R,
    # OperationType.JLE_I,
    # OperationType.JLE_R,
    # OperationType.JGE_I,
    # OperationType.JGE_R,
    # OperationType.CALL_I,
    # OperationType.CALL_R,
    OperationType.DIRECTIVE,
    OperationType.RET,
    OperationType.HALT,
]


def main():
    if (d := "-d" in sys.argv):
        sys.argv.remove("-d")

    if (x := "-x" in sys.argv):
        sys.argv.remove("-x")

    if len(sys.argv) <= 1:
        usage()
        exit(1)

    input_file = sys.argv[1]
    output_file = os.path.splitext(input_file)[0]

    start = time.time()

    tokens = list(lexer_tokenize_file(input_file))
    # if d:
    #     for token in tokens:
    #         postfix = f":{repr(token.val)}" if token.val is not None else ""
    #         report_note(str(token.typ) + postfix, loc=token.loc, fd=sys.stdout)
    #     print()

    p1 = pass1(tokens)
    p2 = pass2(p1)
    p3 = pass3(p2)
    p4 = pass4(*p3)
    ops = parse_operations(p4)
    bytecode = build(ops)

    with open(output_file, "wb") as f:
        f.write(bytecode)

    if d:
        previous = None
        for op in ops:
            operands = "".join(str(operand).ljust(6) for operand in op.val)

            # Separator to help distinguish between some parts of code
            if previous in DEBUG_SEPARATOR and op.typ != previous:
                sys.stdout.write("\n")

            report_note(f"\t{str(op.typ):<16}{operands}", op.loc,
                        fd=sys.stdout)
            previous = op.typ


        if len(p3[1]) != 0:
            print()
            longest_label = len(max(p3[1].keys(), key=len))
            previous_value = 0
            for key, value in p3[1].items():
                diff = value - previous_value
                print(f"  {key:<{longest_label}} {value:04x} ({value})",
                      f"(+{diff})" if diff else "")
                previous_value = value
            print()

    if x:
        for i, byte in enumerate(bytecode):
            print(f"0x{byte:02x}", end=", ")
            if (i + 1) % 16 == 0:
                print()
        if len(bytecode) % 16 != 0:
            print()

    end = time.time()
    print(f"Success. {len(bytecode)} bytes. ({end - start:.2f} s)")


if __name__ == "__main__":
    main()

