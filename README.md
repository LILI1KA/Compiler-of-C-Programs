# Compiler of C-like Programs

A compiler pipeline written in C++ for **WLP4**, a compact C-like language. It translates source programs through lexical analysis, LR parsing, semantic/type checking, and optimized MIPS code generation.

The project supports integer and pointer types, local variables, functions, recursion, conditional statements, `while` loops, dynamic arrays, pointer arithmetic, `println`, and `delete[]`.

## Pipeline

```text
.wlp4 source
    │
    ▼
Scanner ──► token stream
    │
    ▼
LR Parser ──► parse tree
    │
    ▼
Type Checker ──► typed parse tree
    │
    ▼
Optimizing Code Generator ──► MIPS assembly
```

Each stage communicates through standard input and standard output, so the components can be executed independently or connected as a Unix pipeline.

## Highlights

- DFA-based lexical analysis with maximal-munch tokenization and integer-range validation.
- Table-driven LR parsing with explicit shift/reduce transitions.
- Semantic analysis for declarations, scopes, function signatures, argument lists, and `int`/`int*` type rules.
- MIPS code generation for control flow, functions, recursion, stack frames, pointers, heap allocation, printing, and array deletion.
- Code-size optimizations including:
  - constant folding and algebraic simplification;
  - tracked-value propagation where control flow permits it;
  - coalescing adjacent stack-pointer updates;
  - omission of unused procedures;
  - selective emission of runtime imports and initialization;
  - specialized code paths for constant multiplication, division, and remainder operations.

## Repository Structure

```text
Compiler-of-C-Programs/
├── src/
│   ├── scanner/       # Source text -> tokens
│   ├── parser/        # Tokens -> parse tree
│   ├── typechecker/   # Parse tree -> typed parse tree
│   └── codegen/       # Typed parse tree -> MIPS assembly
├── examples/
│   ├── recursion.wlp4
│   ├── sort.wlp4
│   └── full_pipeline.wlp4
├── scripts/
│   └── compile.sh
├── Makefile
└── README.md
```

## Supported Language Subset

The accepted language is intentionally smaller than ISO C. Representative constructs include:

```c
int factorial(int n) {
    int result = 0;
    if (n < 1) {
        result = 1;
    } else {
        result = n * factorial(n - 1);
    }
    return result;
}

int wain(int a, int b) {
    return factorial(a);
}
```

Supported features include:

- `int` and `int*` values;
- variable and function declarations;
- function calls and recursion;
- `if`/`else` and `while`;
- arithmetic and comparison operators;
- address-of and pointer dereference;
- pointer arithmetic;
- `new int[...]` and `delete[]`;
- `println(...)`.

## Build

Requirements:

- GNU Make;
- a C++17-compatible compiler such as `g++`.

Build all four stages:

```bash
make
```

The executables are written to `build/`:

```text
build/wlp4scan
build/wlp4parse
build/wlp4type
build/wlp4gen
```

Remove generated executables with:

```bash
make clean
```

## Compile an Example

The helper script builds the compiler when necessary and runs the complete source-to-assembly pipeline:

```bash
bash scripts/compile.sh examples/recursion.wlp4 > build/recursion.asm
```

More representative inputs are available in `examples/`:

- `recursion.wlp4`: function calls and recursive factorial;
- `sort.wlp4`: recursive quicksort-style logic, arrays, and pointer arithmetic;
- `full_pipeline.wlp4`: control flow, functions, printing, and dynamic allocation.

## Output and Runtime Dependencies

The final stage emits MIPS assembly. Producing a linked executable requires a compatible MIPS assembler/linker and runtime implementations for imported routines such as `print`, `init`, `new`, and `delete`. Those external binaries are not included in this repository.

## Notes

- This is a compiler for a deliberately simplified C-like language, not a full ISO C compiler.
- The scanner reads `src/scanner/dfa.txt`; `scripts/compile.sh` selects the correct working directory automatically.
- The source preserves the original implementation style and data structures used while developing the compiler.

