#!/bin/bash

set -e

SYSTEM_PROGRAM_PUBKEY_C_ARRAY="{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}"
VOTE_PROGRAM_PUBKEY_C_ARRAY="{7,97,72,29,53,116,116,187,124,77,118,36,235,211,189,179,216,53,94,115,209,16,67,252,13,163,83,128,0,0,0,0}"
CLOCK_SYSVAR_PUBKEY_C_ARRAY="{6,167,213,23,24,199,116,201,40,86,99,152,105,29,94,182,139,94,184,163,155,75,109,92,115,85,91,33,0,0,0,0}"
SELF_PROGRAM_PUBKEY_C_ARRAY="{13,185,248,61,2,126,145,5,136,115,112,219,250,155,196,91,249,117,61,215,63,216,69,52,172,123,221,147,224,201,129,197}"

$SDK_ROOT/bpf/dependencies/bpf-tools/llvm/bin/clang                                       \
    -fno-builtin                                                                          \
    -fno-zero-initialized-in-bss                                                          \
    -fno-data-sections                                                                    \
    -std=c2x                                                                              \
    -I$SDK_ROOT/bpf/c/inc                                                                 \
    -O3                                                                                   \
    -target bpf                                                                           \
    -fPIC                                                                                 \
    -march=bpfel+solana                                                                   \
    -I$SOURCE_ROOT/program                                                                \
    -o program.po                                                                         \
    -c $SOURCE_ROOT/program/entrypoint.c                                                  \
    -DSYSTEM_PROGRAM_PUBKEY_ARRAY="$SYSTEM_PROGRAM_PUBKEY_C_ARRAY"                        \
    -DVOTE_PROGRAM_PUBKEY_ARRAY="$VOTE_PROGRAM_PUBKEY_C_ARRAY"                            \
    -DCLOCK_SYSVAR_PUBKEY_ARRAY="$CLOCK_SYSVAR_PUBKEY_C_ARRAY"                            \
    -DSELF_PROGRAM_PUBKEY_ARRAY="$SELF_PROGRAM_PUBKEY_C_ARRAY"

$SDK_ROOT/bpf/dependencies/bpf-tools/llvm/bin/ld.lld                                      \
    -z notext                                                                             \
    -shared                                                                               \
    --Bdynamic                                                                            \
    $SOURCE_ROOT/program/fixed_bpf.ld                                                     \
    --entry entrypoint                                                                    \
    -o program.so                                                                         \
    program.po

rm program.po

strip -s -R .comment --strip-unneeded program.so
