SDK_ROOT?=$(shell echo ~/.local/share/solana/install/active_release/bin/sdk)

program.so: program/entrypoint.c build_program.sh program-key.json
	SDK_ROOT=$(SDK_ROOT) SOURCE_ROOT=. ./build_program.sh

build_program.sh: make_build_program.sh
	./make_build_program.sh program-key.json > $@

.PHONY: test
test:
	SOURCE=`pwd` ./test/test.sh
