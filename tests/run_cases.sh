#!/usr/bin/env bash
# Usage: run_cases.sh <engine> <folder>...
# Automates the tests for the reference engine

set -o pipefail

engine="$1"
shift
failed=0
err=$(mktemp)

for folder in "$@"; do
    for case_in in "$folder"/*.in; do
        expected="${case_in%.in}.out"
        if diff_output=$("$engine" < "$case_in" 2> "$err" | diff "$expected" -) && [ ! -s "$err" ]; then
            echo "PASS $case_in"
        else
            echo "FAIL $case_in"
            echo "$diff_output"
            cat "$err"
            failed=$((failed + 1))
        fi
    done
done

rm -f "$err"
echo "$failed failed"
[ "$failed" -eq 0 ]