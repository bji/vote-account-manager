
# Vote Account Manager Program

## Introduction

This repository implements the Vote Account Manager on-chain program.  It implements a
controlled mechanism for managing a Solana vote account with enhanced security and
support features.


## Location

The Vote Account Manager program is published at the following address:

`vamp1eWV3zyWNiRwC7rdiBznrN8nB8ML8uXGk2kg7cU`


## Verifying

The program is intended to be securely verifiable by any user of the program.  To do
so, the following command line utilities are needed:

a. The solana command line program, available from Solana Labs: [https://docs.solana.com/cli/install-solana-cli-tools](https://docs.solana.com/cli/install-solana-cli-tools).

b. Docker, available here: [https://www.docker.com/](https://www.docker.com).

c. git, available here: [https://git-scm.com/](https://git-scm.com).

The following steps may be used to verify the program:

1. Check to make sure that the Vote Account Manager program is non-upgradeable.  This is
   important because if the program is upgradeable, then it can be changed at any time
   and verification using these steps is only valid until it is changed.  To check:

   ```$ solana program show vamp1eWV3zyWNiRwC7rdiBznrN8nB8ML8uXGk2kg7cU```

   The result must include the line:

    ```Authority: none```

    This line indicates that there is no one authorized to upgrade the program, making it
    non-upgradeable.

    You may also check the program on the solana explorer:

    [https://explorer.solana.com/address/vamp1eWV3zyWNiRwC7rdiBznrN8nB8ML8uXGk2kg7cU](https://explorer.solana.com/address/vamp1eWV3zyWNiRwC7rdiBznrN8nB8ML8uXGk2kg7cU)

    The following will indicate that the program is not upgradeable:

    ```Upgradeable         No```
   
2. Fetch the contents of the published on-chain Solana program:

   ```$ solana program dump vamp1eWV3zyWNiRwC7rdiBznrN8nB8ML8uXGk2kg7cU vote-account-manager.so```

3. Compute the sha256sum of the Vote Account Manager program:

   ```$ sha256sum vote-account-manager.so```

4. The resulting SHA-256 checksum should be:

   `8a21322d1247da8d26db86a328648e185642ff3974ada63d897f42769a8f46c6`

   This is the signature of the contents of the Vote Account Manager program.

5. Checkout the Vote Account Manager source code:

   ```$ git clone https://github.com/bji/vote-account-manager.git```

6. Inspect the Vote Account Manager source code that was just checked out.  Ensure that it does
   what it promises to do.  This requires software development expertise; if you don't have
   such expertise, find someone who does and who can help you.

7. Reproduce the build:

   ```$ docker build -t vote-account-manager vote-account-manager```

   This build will succeed only if the Vote Account Manager program built from the source is
   identical to the on-chain program.  To verify this:

   a. Inspect the vote-account-manager/Dockerfile.  Note that it only uses standard tooling.  Note
      also that it checks out the same Vote Account Manager code that you checked out in step (5)
      so you can be sure that the code you have inspected is the code that was built.

   b. Note also that the Dockerfile builds using the `build_program.sh` script.  Inspect this
      file to ensure that it builds the program as you would expect.  Note that it includes
      hardcoded pubkeys and seeds; you can see how these were produced by looking at the
      make_build_program.sh script.

   c. Note that the Dockerfile will fail to build if the command that compiles the Vote Account
      Manager program results in a file with a SHA-256 sum different than the one that was
      shown to be the signature of the on-chain program in step (1).

8. You have now inspected all files that go into building the Vote Account Manager program, have
   verified that only open-source and standard tooling was used to build it, have verified that
   the source code you have access to is what the program was built from, and have verified
   that the on-chain program was built exactly from that source.  And because the program
   is non-upgradeable, it can never change and thus you know that the program will always be
   exactly that derived from the source you have inspected.


## Testing

If you like, you can run the tests which verify the functionality of the Vote Account Manager
program.  To do so, you will need solxact, available on github: [https://github.com/bji/solxact](https://github.com/bji/solxact).

You should verify the fidelity of these programs - the source is open and available.

Then, run the following command:

```$ SOURCE=`pwd`/vote-account-manager ./vote-account-manager/test/test.sh```

This will take a while to run to completion - about half an hour.  This is because it runs many
tests and it is conservative in waiting for commands to complete successfully before proceeding
to the next command.

You can inspect all of the tests that were run by looking at the files in the `test` directory.


## Issuing Manual Transactions


The vote-account-manager repository contains a script that can be used to interact with the
Vote Account Manager program.  This script is present in the scripts directory and is named `vamp`.
The `solxact` utility is required to use this script.


## Inspecting On-Chain State


The show.sh program in the scripts directory can be used to dump into JSON format all
on-chain account data associated with the Vote Account Manager program.


## License

The Vote Account Manager program and all contents of this repository are Copyright 2022
by Shinobi Systems LLC, and licensed under the GNU Lesser General Public License Version 3.
