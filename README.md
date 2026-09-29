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

Produces the compiler at `build/bin/pragc`, plus the test binaries.

## Testing

Build first (see above), then:

```bash
ctest --test-dir build --output-on-failure
```

Or from inside `build/`, just `ctest`. `--output-on-failure` prints the Catch2
output for any test that fails; without it you only see pass/fail.

| ctest name    | What it covers                                           |
|---------------|----------------------------------------------------------|
| `Lexer`       | tokenising `.pragma` source                              |
| `Parser`      | building the AST                                         |
| `Typechecker` | type errors and parallel-safety (accept/reject fixtures) |
| `Generator`   | emitted C code                                           |
| `Pragc`       | the `pragc` CLI end to end                               |
| `Runtime`     | generated OpenMP code actually runs with multiple threads |

Useful variants:

```bash
ctest --test-dir build -R Lexer            # run one test by name (regex)
ctest --test-dir build -V                  # verbose: show all output
./build/bin/TestTypechecker "<test name>"  # run one Catch2 TEST_CASE directly
```

Tests run from the repo root, so fixture paths like `tests/lex_files/...` resolve.
Run the Catch2 binaries directly from the repo root too.

## Usage

```bash
pragc <input.pragma> [--compile] [--run]
```

- (no flags) — emits `output.c`
- `--compile` — also compiles `output.c` to `output.o` (`gcc -std=c99 -fopenmp -c`)
- `--run` — builds `output.c` into `output_bin` and runs it. Needs code
  outside any function (see below).

A file can hold several functions, which can call each other. Code outside
any function becomes `main`, and `out <expr>` prints a value:

```
fn square:int (x:int){
    return x*x
}

total = 0
for i in 0..10 {
    total += square(i)
}
out total
```

A file with only functions has no `main`; it can still be compiled with
`--compile` and linked from C. See `examples/kernels.pragma`.

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
