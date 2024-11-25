#!/usr/bin/python3

import sys
from typing import *
from dataclasses import dataclass

Loc = Tuple[str, int]
Line = Tuple[int, str]

def splitlines(text: str) -> List[Line]:
  return [(i + 1, line) for i, line in enumerate(text.splitlines())]

@dataclass(frozen=True)
class FatalError(Exception):
  msg: str
  loc: Loc

def report_error(error: FatalError) -> None:
  file, line = error.loc
  location: str = f"{file} [{line}]:"
  message: str = f"fatal-error: {error.msg}"

  sys.stderr.write(f"{location}\n")
  sys.stderr.write(f"{message}\n")

MACRO_PREFIX: Final[str] = "%"
MACRO_DEFINE: Final[str] = MACRO_PREFIX + "define"
MACRO_END: Final[str] = MACRO_PREFIX + "end"

def assert_macro_define(fragments: List[str], loc: Loc,
                        macro_name: Optional[str]) -> None:
  if macro_name is not None:
    raise FatalError(f"{MACRO_DEFINE} inside {MACRO_DEFINE}", loc)

  elif len(fragments) != 2:
    raise FatalError(f"{MACRO_DEFINE} requires exactly 1 fragment", loc)

def assert_macro_end(fragments: List[str], loc: Loc,
                     macro_name: Optional[str]) -> None:
  if macro_name is None:
    raise FatalError(f"{MACRO_END} without {MACRO_DEFINE}", loc)

  elif len(fragments) != 1:
    raise FatalError(f"{MACRO_END} requires exactly 0 fragments", loc)

def assert_macro_expand(fragments: List[str], loc: Loc,
                        macro_name: Optional[str]) -> None:
  if macro_name is not None:
    raise FatalError(f"{MACRO_PREFIX} inside {MACRO_DEFINE}", loc)
  
  elif len(fragments) != 1:
    raise FatalError(f"{MACRO_END} requires exactly 0 fragments", loc)

def preprocess(lines: List[Line], file: str) -> List[Line]:
  macros: Dict[str, List[Line]] = {}
  macro_name: Optional[str] = None
  macro_buffer: List[Line] = []
  macro_begin: Optional[Loc] = None

  result: List[Line] = []
 
  for i, line in lines:
    loc: Loc = (file, i)

    stripped: str = line.strip()
    fragments: List[str] = stripped.split()

    if stripped.startswith(MACRO_END):
      assert_macro_end(fragments, loc, macro_name)
      macros[cast(str, macro_name)] = macro_buffer
      macro_name, macro_buffer = None, []

    elif stripped.startswith(MACRO_DEFINE):
      assert_macro_define(fragments, loc, macro_name)
      macro_name = fragments[1]
      macro_begin = loc

      if macro_name in macros:
        raise FatalError(f"redefinition of macro {macro_name}", loc)

    elif stripped.startswith(MACRO_PREFIX):
      assert_macro_expand(fragments, loc, macro_name)
      name: str = fragments[0][1:]

      if name not in macros:
        raise FatalError(f"undefined macro {name}", loc)

      result.extend(macros[name])

    elif macro_name is not None:
      macro_buffer.append((i, line))

    else:
      result.append((i, line))

  if macro_name is not None:
    raise FatalError(f"{MACRO_DEFINE} without {MACRO_END}",
                     cast(Loc, macro_begin))
  
  return result

def main() -> None:
  path: str = sys.argv[1]
  with open(path, 'r') as f:
    text: str = f.read()

  lines: List[Line] = splitlines(text)
  processed = preprocess(lines, path)

  for i, line in processed:
    print(i, line)

if __name__ == "__main__":
  try:
    main()
  except FatalError as e:
    report_error(e)

