# pragma

[![CI](https://github.com/Tangy-Cheese/pragma/actions/workflows/ci.yml/badge.svg)](https://github.com/Tangy-Cheese/pragma/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/github/license/Tangy-Cheese/pragma)](LICENSE)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)

Pragma is a DSL for heavy numerical/HPC code that compiles to parallelised
OpenMP C. "C without pointers, with automatic parallelism" — you write
readable numerical functions, `pragc` handles memory and OpenMP pragma
emission.

Source files use the `.pragma` extension. Full language spec: [spec.md](spec.md).

## Pipeline

```
source.pragma → Lexer → Tokens → Parser → AST → TypeChecker → CodeGenerator → output.c
```

## Requirements

- CMake 3.20 or newer
- gcc with OpenMP support
- Catch2 v3, installed system-wide (v2 will not work)

## Building

```bash
cmake -B build
cmake --build build
```

Produces `build/bin/pragc` plus test binaries (`TestLexer`, `TestParser`,
`TestTypechecker`, `TestGenerator`, `TestPragc`), run via `ctest` from `build/`.

## Usage

```bash
pragc <input.pragma> [--compile] [--run]
```

- (no flags) — emits `output.c`
- `--compile` — also compiles `output.c` to `output.o` (`gcc -std=c99 -fopenmp -c`)
- `--run` — synthesizes a `driver.c` with a `main()` calling the parsed
  function with default args, links it against `output.c`, and runs the
  binary. Only scalar (`int`/`float`/`double`) params and return types are
  supported — the parser produces one function per file, not a full program.

## Example

`examples/example.pragma`:

```
fn sumto:int (n:int){
    sum = 0
    for i in 0..n {
        sum = sum+i
    }
    return sum
}
```

## Status

Early / actively developed. Lexer, parser, typechecker, and generator are
implemented (`src/`); CLI surface (`--compile`, `--run`) is a subset of what's
described in `spec.md` — flags like `--build`, `-o`, `--dump-ast` aren't wired
up yet.

## License

MIT — see [LICENSE](LICENSE).
