#!/usr/bin/env bash
# Usage: run_cases.sh <engine> <folder>
# Automates the tests for the reference engine

engine="$1"
shift
failed=0

for folder in "$@"; do
    for case_in in "$folder"/*.in; do
        expected="${case_in%.in}.out"
        if diff_output=$("$engine" < "$case_in" | diff "$expected" -); then
            echo "PASS $case_in"
        else
            echo "FAIL $case_in"
            echo "$diff_output"
            failed=$((failed + 1))
        fi
    done
done

echo "$failed failed"
[ "$failed" -eq 0 ]