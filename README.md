# pragma

[![CI](https://github.com/Tangy-Cheese/pragma/actions/workflows/ci.yml/badge.svg)](https://github.com/Tangy-Cheese/pragma/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/github/license/Tangy-Cheese/pragma)](LICENSE)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)

Pragma is a small language for numerical/HPC code that compiles to OpenMP C.
"C without pointers, with automatic parallelism": you write readable numerical
code, and `pragc` handles memory allocation and emits the OpenMP pragmas,
including reduction clauses. It also rejects parallel loops it can't prove
race-free.

Source files use the `.pragma` extension. Language spec: [spec.md](spec.md).

```
source.pragma → Lexer → Parser → TypeChecker → CodeGenerator → output.c
```

## Quick start

Requirements: CMake 3.20+, gcc with OpenMP, Catch2 v3 (system-wide; v2 won't work).

```bash
cmake -B build
cmake --build build
./build/bin/pragc examples/numerics.pragma --run
```

To run it as plain `pragc` from anywhere, install it onto your `PATH`:

```bash
cmake --install build --prefix ~/.local    # just you: ~/.local/bin/pragc
sudo cmake --install build                 # everyone: /usr/local/bin/pragc
```

`~/.local/bin` is on `PATH` by default on most Linux distros. If `pragc` isn't
found, add `export PATH="$HOME/.local/bin:$PATH"` to your `~/.bashrc`.
Re-run the install after rebuilding to pick up changes. `pragc` writes
`output.c` (and `output_bin` with `--run`) into the directory you run it from.

## The language

```
fn square:int (x:int){
    return x*x
}

fn sum_squares:int (n:int){
    s = 0
    parallel i in 0..n {
        s += square(i)
    }
    return s
}

n = 10
out sum_squares(n)
```

This prints `285`. The `parallel` loop becomes
`#pragma omp parallel for reduction(+:s)`, and the last two lines, being outside
any function, become `main`.

- **Functions:** `fn name:return_type (param:type, ...){ ... return expr }`. Several per file, callable in any order.
- **Variables:** no declarations; the type comes from the first assignment (`s = 0` makes `s` an `int`).
- **Types:** `int`, `float`, `double`. Literals: `1` is `int`, `1.5` is `float`.
- **Arrays:** `a = alloc(float, n)`, indexed with `a[i]`. Use `a[i*n+j]` for 2D.
- **Loops:** `for i in 0..n { }` runs sequentially; `parallel i in 0..n { }` runs in parallel with OpenMP. Ranges exclude the end value.
- **Assignment:** `=`, `+=`, `-=`, `*=`, `/=`.
- **Output:** `out <expr>` prints an `int` or `float`/`double`.

A `parallel` loop is accepted only when it's provably safe:
- A variable that already exists outside the loop may only be updated with `+=` or `*=`; these become OpenMP reductions.
- Array writes must be indexed by exactly the loop variable, e.g. `a[i] = ...`.
- Arrays that are only read may use any index.

## Usage

```bash
pragc <input.pragma> [--compile] [--run]
```

- (no flags): writes `output.c`
- `--compile`: also compiles it to `output.o`
- `--run`: builds `output_bin` and runs it. Needs code outside any function, which becomes `main`.

A file with only functions has no `main`. It can still be compiled with
`--compile` and linked from C, as `tests/runtime/` does.

## Examples

- [`examples/numerics.pragma`](examples/numerics.pragma): a full program.
  Midpoint-rule integration, compound growth (`*=` reduction), Fibonacci,
  1D heat diffusion over time, and matrix multiply + trace. Output is
  identical with 1 and 8 threads.
- [`examples/kernels.pragma`](examples/kernels.pragma): eight small kernels
  (dot product, saxpy, stencil, prefix sum, ...), printed from `main`.

## Testing

```bash
ctest --test-dir build --output-on-failure
```

| ctest name    | What it covers                                            |
|---------------|-----------------------------------------------------------|
| `Lexer`       | tokenising source                                         |
| `Parser`      | building the syntax tree                                  |
| `Typechecker` | type errors and parallel-safety rules                     |
| `Generator`   | emitted C code                                            |
| `Pragc`       | the `pragc` command end to end, plus a timing benchmark   |
| `Runtime`     | generated code gives the right answer at 1 and 8 threads  |

Run a single test with `ctest --test-dir build -R Lexer`, or a single Catch2
case with `./build/bin/TestParser "<test name>"`. Run test binaries from the repo
root, because they load input files from `tests/lex_files/`.

## Current limits

Known gaps, roughly in order of how often you'll hit them:

- No `if`/`else` yet. The keywords are reserved, but the parser doesn't handle them.
- No comments: `#` or `//` in a source file is a syntax error.
- No parentheses in expressions: write `r*a - r*b`, not `r*(a - b)`.
- No conversion between `int` and `float`, e.g. `i*h` with `i:int`, `h:float` is a type error.
- Functions can't take arrays as parameters.
- Identifiers can't contain digits (`x2` lexes as `x` then `2`).
- A function with no `return` crashes the compiler.
- Parse errors are printed but don't stop compilation.
- Arrays are never freed in the generated C.

## License

MIT, see [LICENSE](LICENSE).
