#!/usr/bin/env bash

set -euo pipefail

build_dir="${1:?build directory is required}"

compiler_path() {
    local cache_name="$1"
    local matches

    matches="$(cmake -LA -N "$build_dir" | awk -v cache_name="$cache_name" '
        $0 ~ ("^" cache_name ":") {
            sub(/^[^=]*=/, "")
            print
        }
    ')"

    test "$(printf '%s\n' "$matches" | awk 'NF { count += 1 } END { print count + 0 }')" -eq 1
    printf '%s\n' "$matches"
}

verify_compiler() {
    local cache_name="$1"
    local xcrun_name="$2"
    local selected_compiler
    local selected_identity
    local reference_identity

    selected_compiler="$(compiler_path "$cache_name")"
    test -x "$selected_compiler"
    selected_identity="$("$selected_compiler" --version | sed -n '1p')"
    reference_identity="$(xcrun "$xcrun_name" --version | sed -n '1p')"
    test -n "$selected_identity"
    test -n "$reference_identity"
    printf '%s\n' "$selected_identity" | grep -q '^Apple clang version 17'
    test "$selected_identity" = "$reference_identity"
}

verify_compiler CMAKE_C_COMPILER clang
verify_compiler CMAKE_CXX_COMPILER clang++
