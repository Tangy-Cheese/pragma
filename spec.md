# Pragma — Full Project Specification

## What Is Pragma

Pragma is a domain-specific language (DSL) for numerical HPC programming that compiles to
parallelised C with OpenMP. The pitch in one sentence: "C without pointers, with automatic
parallelism." Scientists write clean, readable numerical code and the compiler handles all
malloc/free, OpenMP pragma emission, and parallelisation decisions automatically.

It is NOT:
- A Python transpiler (Cython already does that)
- A general purpose language
- A research novelty

It IS:
- A purpose-built DSL for HPC numerical code
- A source-to-source compiler emitting human-readable standalone C
- A tool that automatically infers correct OpenMP clauses

The compiler is called `pragc`. Source files use the `.pg` extension.

---

## The Compiler Pipeline

```
source.pg → Lexer → Tokens → Parser → AST → Analysis → Codegen → output.c
```

Then the user runs:
```bash
gcc -O2 -fopenmp output.c -o program -lm
```

Or uses the `--build` flag to do it in one step.

---

## Compiler CLI — `pragc`

```bash
pragc heat.pg                        # outputs heat.c
pragc heat.pg -o solver.c            # specify output file
pragc heat.pg --build                # outputs heat.c AND compiles to binary
pragc heat.pg --build -o heat        # named binary
pragc heat.pg --build --threads 8    # thread count hint
pragc heat.pg --build -O2            # optimisation level passed to GCC (default)
pragc heat.pg --build -O3
pragc heat.pg --dump-ast             # debug: print AST
pragc heat.pg --dump-omp             # debug: show inferred OMP clauses
pragc heat.pg --verbose              # show every compiler decision
```

The `.c` file is ALWAYS emitted — it is a first-class output, never a temp file.
`--build` produces BOTH the `.c` file and the compiled binary.

---

## Language Syntax

### Functions
```
fn name(param: type, param: type) -> return_type {
    ...
}
```

### Variable Declaration (type inferred from value for locals)
```
x = 5           # inferred int
y = 3.14        # inferred float
flag = true     # inferred bool
```

### Constants
```
const PI    = 3.14159265
const ALPHA = 0.01
```

### Array Allocation (type must be explicit)
```
a = alloc(float, n)
b = alloc(int, n)
```

### Array Access
```
a[i]
a[i * m + j]    # flat 2D indexing
```

### Arithmetic & Logic
```
+ - * / %
+= -= *= /=
== != < > <= >=
&& || !
```

### Control Flow
```
if condition {
} elif condition {
} else {
}
```

### Loops
```
for i in 0..n {
}

for i in start..end {
}

while condition {
}
```

### Parallel Loop
```
parallel i in 0..n {
}

parallel i in 1..n-1 {
}
```

### Return
```
return x
return          # void functions
```

### Comments
```
# this is a comment
```

### Print
```
print("hello")
print(x)
print(u[i])
print("result:", x)
print("step:", t, "res:", residual)
```

Rules for print:
- Pragma infers %d vs %f from type automatically
- print inside a parallel block is a COMPILE ERROR (race on stdout)
- Emits printf with \n appended automatically

### Built-in Math
```
sin(x)
cos(x)
sqrt(x)
abs(x)
min(a, b)
max(a, b)
```

### Extern Declarations (for non-builtin C math functions)
```
extern cbrt(x: float) -> float
extern erf(x: float) -> float
```

### Break / Continue
```
for i in 0..n {
    if a[i] < 0.0 {
        break
    }
}
```

---

## Type System

| Pragma type | C type      |
|-------------|-------------|
| int         | int         |
| float       | double      |
| bool        | int (0/1)   |
| float[]     | double*     |
| int[]       | int*        |
| void        | void        |

Rules:
- Function parameters ALWAYS require explicit type annotations
- Function return types ALWAYS require explicit annotation
- Local scalar variables are inferred from their assigned value
- alloc() requires explicit type: `alloc(float, n)` not `alloc(n)`
- float maps to double in C — double precision is the HPC default

---

## Memory Management

Pragma handles all malloc/free automatically. Rules:

1. **Allocator owns** — whatever scope calls alloc() is responsible for freeing
2. **Borrowed parameters** — functions receiving float[] as a parameter NEVER free it
3. **Returned arrays** — ownership transfers to the caller; Pragma suppresses free at
   the return site and emits free at the caller's scope end
4. **Early returns** — Pragma emits free for all live allocations before EVERY return path
5. **alloc inside parallel block** — COMPILE ERROR

```
fn solve(n: int) -> float[] {
    u = alloc(float, n)
    temp = alloc(float, n)  # freed here — local, not returned
    # ...
    return u                # u NOT freed here — ownership transferred
}

fn main() -> void {
    result = solve(100)
    # ...
}                           # Pragma emits free(result) here
```

---

## OpenMP Automatic Inference

Pragma ONLY emits OpenMP it can PROVE is correct. Rules:

| Pattern detected | Clause emitted |
|-----------------|----------------|
| Unique write index a[i] | none needed — just parallel for |
| scalar += ... | reduction(+:scalar) |
| scalar *= ... | reduction(*:scalar) |
| Clean nested loops, no cross-iteration dependency | collapse(2) |
| Read from a[i-1] or a[i+1] in nested loop | suppress collapse, warn |
| Scalar write with no reduction pattern | COMPILE ERROR with suggestion |
| print() inside parallel block | COMPILE ERROR |

Clauses Pragma emits: reduction, collapse, private, firstprivate, num_threads, schedule
Clauses Pragma does NOT attempt: GPU offload, task dependencies, SIMD, target

---

## Compile Errors Pragma Catches

```
# alloc inside parallel block
parallel i in 0..n {
    a = alloc(float, n)     # ERROR: alloc not permitted in parallel block
}

# unresolvable scalar write
parallel i in 0..n {
    result = a[i] * b[i]    # ERROR: scalar 'result' written in parallel loop
                             # with no reduction pattern. Did you mean result += ?
}

# type mismatch
x: int = 3.14               # ERROR: cannot assign float literal to int

# print in parallel
parallel i in 0..n {
    print(a[i])             # ERROR: print not permitted in parallel block
}
```

---

## Example — Full Pragma Program

```
const PI = 3.14159265

fn dot(a: float[], b: float[], n: int) -> float {
    sum = 0.0
    parallel i in 0..n {
        sum += a[i] * b[i]
    }
    return sum
}

fn scale(a: float[], n: int, factor: float) -> void {
    parallel i in 0..n {
        a[i] = a[i] * factor
    }
}

fn clamp(a: float[], out: float[], n: int, lo: float, hi: float) -> void {
    parallel i in 0..n {
        if a[i] < lo {
            out[i] = lo
        } elif a[i] > hi {
            out[i] = hi
        } else {
            out[i] = a[i]
        }
    }
}

fn main() -> void {
    n      = 1000
    factor = 2.5

    a = alloc(float, n)
    b = alloc(float, n)
    c = alloc(float, n)

    for i in 0..n {
        a[i] = sin(PI * i / n)
        b[i] = cos(PI * i / n)
    }

    scale(a, n, factor)
    clamp(a, c, n, -1.0, 1.0)

    result = dot(a, b, n)
    print("dot product:", result)

    m   = 50
    mat = alloc(float, n * m)

    parallel i in 0..n {
        for j in 0..m {
            mat[i * m + j] = a[i] * b[j]
        }
    }
}
```

---

## Example — Crank-Nicolson Heat Equation

```
fn build_rhs(u: float[], rhs: float[], n: int, r: float,
             u_left: float, u_right: float) -> void {
    parallel i in 1..n-1 {
        rhs[i] = (r/2.0)*u[i-1] + (1.0 - r)*u[i] + (r/2.0)*u[i+1]
    }
    rhs[1]   = rhs[1]   + (r/2.0) * u_left
    rhs[n-2] = rhs[n-2] + (r/2.0) * u_right
}

fn thomas_solve(lower: float[], diag: float[], upper: float[],
                d: float[], x: float[], n: int) -> void {
    c_prime = alloc(float, n)
    d_prime = alloc(float, n)

    c_prime[0] = upper[0] / diag[0]
    d_prime[0] = d[0]     / diag[0]

    for i in 1..n {
        denom      = diag[i] - lower[i] * c_prime[i-1]
        c_prime[i] = upper[i] / denom
        d_prime[i] = (d[i] - lower[i] * d_prime[i-1]) / denom
    }

    x[n-1] = d_prime[n-1]

    for i in 0..n-2 {
        j    = n-2-i
        x[j] = d_prime[j] - c_prime[j] * x[j+1]
    }
}

fn solve_heat(n: int, nt: int, alpha: float,
              u_left: float, u_right: float) -> float[] {
    dx = 1.0 / (n - 1)
    dt = 0.001
    r  = alpha * dt / (dx * dx)

    u     = alloc(float, n)
    u_new = alloc(float, n)
    lower = alloc(float, n)
    diag  = alloc(float, n)
    upper = alloc(float, n)
    rhs   = alloc(float, n)

    parallel i in 0..n {
        u[i] = sin(3.14159265 * i * dx)
    }
    u[0]   = u_left
    u[n-1] = u_right

    parallel i in 1..n-1 {
        lower[i] = -r / 2.0
        diag[i]  =  1.0 + r
        upper[i] = -r / 2.0
    }

    for t in 0..nt {
        build_rhs(u, rhs, n, r, u_left, u_right)
        thomas_solve(lower, diag, upper, rhs, u_new, n-2)
        parallel i in 1..n-1 {
            u[i] = u_new[i]
        }
        u[0]   = u_left
        u[n-1] = u_right
    }

    return u
}

fn main() -> void {
    result = solve_heat(100, 1000, 0.01, 0.0, 0.0)
    print("midpoint:", result[50])
}
```

---

## Project Structure

```
pragma/
│
├── CMakeLists.txt
├── README.md
│
├── src/
│   ├── main.cpp
│   ├── lexer/
│   │   ├── lexer.hpp
│   │   ├── lexer.cpp
│   │   ├── token.hpp
│   │   └── token.cpp
│   ├── parser/
│   │   ├── parser.hpp
│   │   └── parser.cpp
│   ├── ast/
│   │   ├── ast.hpp
│   │   ├── ast.cpp
│   │   └── ast_printer.cpp
│   ├── analysis/
│   │   ├── type_checker.hpp
│   │   ├── type_checker.cpp
│   │   ├── dependency_analyser.hpp
│   │   ├── dependency_analyser.cpp
│   │   ├── race_detector.hpp
│   │   └── race_detector.cpp
│   ├── omp/
│   │   ├── omp_analyser.hpp
│   │   └── omp_analyser.cpp
│   ├── memory/
│   │   ├── memory_manager.hpp
│   │   └── memory_manager.cpp
│   ├── codegen/
│   │   ├── codegen.hpp
│   │   └── codegen.cpp
│   └── error/
│       ├── error.hpp
│       └── error.cpp
│
├── examples/
│   ├── heat_equation/
│   │   ├── heat.pg
│   │   ├── heat.py
│   │   ├── heat.c
│   │   └── README.md
│   ├── dot_product/
│   │   ├── dot.pg
│   │   ├── dot.py
│   │   ├── dot.c
│   │   └── README.md
│   ├── matrix_multiply/
│   │   ├── matmul.pg
│   │   ├── matmul.py
│   │   ├── matmul.c
│   │   └── README.md
│   ├── laplace_equation/
│   │   ├── laplace.pg
│   │   ├── laplace.py
│   │   ├── laplace.c
│   │   └── README.md
│   └── wave_equation/
│       ├── wave.pg
│       ├── wave.py
│       ├── wave.c
│       └── README.md
│
└── tests/
    ├── test_lexer.cpp
    ├── test_parser.cpp
    ├── test_typechecker.cpp
    ├── test_omp.cpp
    ├── test_memory.cpp
    ├── test_codegen.cpp
    └── test_end_to_end.cpp
```

---

## Estimated LOC Per Component

| Component                  | LOC  |
|---------------------------|------|
| lexer.cpp                 | 350  |
| token.hpp                 | 100  |
| parser.cpp                | 700  |
| ast.hpp                   | 300  |
| type_checker.cpp          | 500  |
| dependency_analyser.cpp   | 300  |
| race_detector.cpp         | 150  |
| omp_analyser.cpp          | 350  |
| memory_manager.cpp        | 200  |
| codegen.cpp               | 600  |
| error.cpp                 | 150  |
| tests                     | 700  |
| **Total**                 | **~3,900** |

---

## Build Order

1. Token definitions and lexer
2. Parser and AST nodes
3. Type checker
4. Codegen for simple cases first (scalars, functions, basic loops)
5. Memory manager
6. OMP analyser
7. Wire everything together
8. Examples and benchmarks

Get dot product compiling end to end before touching anything else.

---

## Explicitly Out of Scope (v1)

- Structs / unions / enums
- Strings
- Dynamic collections (append, pop)
- Classes
- Recursion
- GPU offload (target, teams, map clauses)
- Exception handling
- 2D array sugar a[i,j] — use flat a[i*m+j]
- Multiple return values
- Modules / multiple files

---

## v2 Planned Extensions

- Structs (for grid metadata in complex PDE solvers)
- Multiple return values
- 2D array sugar a[i,j]
- print_fmt for formatted output

---

## README Pitch (use this)

> Pragma feels like C without pointers. That's intentional. The complexity of C that matters
> for HPC — static types, flat arrays, explicit sizing — is preserved. The complexity that
> doesn't — pointer arithmetic, manual memory management, preprocessor boilerplate — is
> handled by the compiler.
>
> Pragma targets numerical HPC workloads where computation is expressed as operations over
> large arrays. This constraint allows the compiler to make strong parallelisation guarantees
> that a general-purpose language cannot.

