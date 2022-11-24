#!/bin/sh

# Builds a build script given the Vote Account Manager program pubkey.
# Requires that the solxact program be in the path.
# The resulting script requires the following variables to be defined:
# SDK_ROOT -- path to the root of the Solana SDK to use for building the program
# SOURCE_ROOT -- path to the Vote Account Manager source

SELF_PROGRAM_PUBKEY=$1

if [ -z "$1" -o -n "$2" ]; then
    echo "Usage: make_build_script.sh <PROGRAM_PUBKEY_OR_KEYPAIR_FILE>"
    exit 1
fi

# pubkeys that are pre-defined
SYSTEM_PROGRAM_PUBKEY=11111111111111111111111111111111
VOTE_PROGRAM_PUBKEY=Vote111111111111111111111111111111111111111
CLOCK_SYSVAR_PUBKEY=SysvarC1ock11111111111111111111111111111111

# input pubkeys
SELF_PROGRAM_PUBKEY=`solxact pubkey $SELF_PROGRAM_PUBKEY`

# pubkeys as JSON byte arrays
SYSTEM_PROGRAM_PUBKEY_ARRAY=`solxact pubkey bytes $SYSTEM_PROGRAM_PUBKEY`
VOTE_PROGRAM_PUBKEY_ARRAY=`solxact pubkey bytes $VOTE_PROGRAM_PUBKEY`
CLOCK_SYSVAR_PUBKEY_ARRAY=`solxact pubkey bytes $CLOCK_SYSVAR_PUBKEY`
SELF_PROGRAM_PUBKEY_ARRAY=`solxact pubkey bytes $SELF_PROGRAM_PUBKEY`

# C versions of pubkey arrays
SYSTEM_PROGRAM_PUBKEY_C_ARRAY=`echo "$SYSTEM_PROGRAM_PUBKEY_ARRAY" | tr '[' '{' | tr ']' '}'`
VOTE_PROGRAM_PUBKEY_C_ARRAY=`echo "$VOTE_PROGRAM_PUBKEY_ARRAY" | tr '[' '{' | tr ']' '}'`
CLOCK_SYSVAR_PUBKEY_C_ARRAY=`echo "$CLOCK_SYSVAR_PUBKEY_ARRAY" | tr '[' '{' | tr ']' '}'`
SELF_PROGRAM_PUBKEY_C_ARRAY=`echo "$SELF_PROGRAM_PUBKEY_ARRAY" | tr '[' '{' | tr ']' '}'`

cat <<EOF
#!/bin/bash

set -e

SYSTEM_PROGRAM_PUBKEY_C_ARRAY="$SYSTEM_PROGRAM_PUBKEY_C_ARRAY"
VOTE_PROGRAM_PUBKEY_C_ARRAY="$VOTE_PROGRAM_PUBKEY_C_ARRAY"
CLOCK_SYSVAR_PUBKEY_C_ARRAY="$CLOCK_SYSVAR_PUBKEY_C_ARRAY"
SELF_PROGRAM_PUBKEY_C_ARRAY="$SELF_PROGRAM_PUBKEY_C_ARRAY"

\$SDK_ROOT/bpf/dependencies/bpf-tools/llvm/bin/clang                                       \\
    -fno-builtin                                                                          \\
    -fno-zero-initialized-in-bss                                                          \\
    -fno-data-sections                                                                    \\
    -std=c2x                                                                              \\
    -I\$SDK_ROOT/bpf/c/inc                                                                 \\
    -O3                                                                                   \\
    -target bpf                                                                           \\
    -fPIC                                                                                 \\
    -march=bpfel+solana                                                                   \\
    -I\$SOURCE_ROOT/program                                                                \\
    -o program.po                                                                         \\
    -c \$SOURCE_ROOT/program/entrypoint.c                                                  \\
    -DSYSTEM_PROGRAM_PUBKEY_ARRAY="\$SYSTEM_PROGRAM_PUBKEY_C_ARRAY"                        \\
    -DVOTE_PROGRAM_PUBKEY_ARRAY="\$VOTE_PROGRAM_PUBKEY_C_ARRAY"                            \\
    -DCLOCK_SYSVAR_PUBKEY_ARRAY="\$CLOCK_SYSVAR_PUBKEY_C_ARRAY"                            \\
    -DSELF_PROGRAM_PUBKEY_ARRAY="\$SELF_PROGRAM_PUBKEY_C_ARRAY"

\$SDK_ROOT/bpf/dependencies/bpf-tools/llvm/bin/ld.lld                                      \\
    -z notext                                                                             \\
    -shared                                                                               \\
    --Bdynamic                                                                            \\
    \$SOURCE_ROOT/program/fixed_bpf.ld                                                     \\
    --entry entrypoint                                                                    \\
    -o program.so                                                                         \\
    program.po

rm program.po

strip -s -R .comment --strip-unneeded program.so
EOF
