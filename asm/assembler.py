#!/usr/bin/python3

import sys
from typing import *
from dataclasses import dataclass
from enum import Enum, auto, unique

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
        return f"({self.typ}: {self.val})"


@unique
class DirectiveType(Enum):
    DW = auto()


DIRECTIVES = {item.name.lower(): item for item in DirectiveType}


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
    """Generate labels"""
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
    print(*[(key, value) for key, value in labels.items()], sep='\n')


if __name__ == "__main__":
    main()

