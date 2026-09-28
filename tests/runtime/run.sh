#!/usr/bin/env bash
# Runtime test: compile pragma programs to C, link each with main.c, run with
# 1 and 8 OpenMP threads, and require the result to equal the sequential answer.
#
# usage: run.sh <path-to-pragc>   (run from the repo root)

set -euo pipefail

PRAGC=$(realpath "$1")
SRC=$(pwd)/tests/runtime
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT

N=20000

# check <program> <function name> <expected result>
check(){
    local program=$1 kernel=$2 expected=$3

    mkdir "$WORK/$program"

    # pragc writes output.c into the current directory, hence the subshell.
    (
        cd "$WORK/$program"
        "$PRAGC" "$SRC/$program.pragma" > /dev/null
        gcc -std=c99 -O2 -fopenmp -DKERNEL="$kernel" "$SRC/main.c" output.c -o prog
    )

    for threads in 1 8; do
        echo "$program: OMP_NUM_THREADS=$threads"
        OMP_NUM_THREADS=$threads "$WORK/$program/prog" "$N" "$expected"
    done
}

# Expected values come from shell arithmetic, independent of the generated code.
check reduce sumall $(( N * (N - 1) / 2 ))
check map mapsum $(( N * (N - 1) ))

echo "runtime test passed"
