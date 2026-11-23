#!/bin/sh
# Builds the engine and checks the seven worked examples byte for byte.
# Usage: sh run_examples.sh [path/to/matching_engine.cpp]
set -eu
here=$(cd "$(dirname "$0")" && pwd)
src=${1:-"$here/../matching_engine.cpp"}
bin=$(mktemp)
trap 'rm -f "$bin"' EXIT
g++ -std=c++20 -O0 -Wall -Wextra -Wpedantic -Werror "$src" -o "$bin"
fail=0
for n in 1 2 3 4 5 6 7; do
    if "$bin" < "$here/example$n.in" | cmp -s - "$here/example$n.expected"; then
        echo "example $n: ok"
    else
        echo "example $n: MISMATCH"
        fail=1
    fi
done
exit $fail
