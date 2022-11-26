
# Vote Account Manager Program

## Introduction

This repository implements the Vote Account Manager on-chain program.  It implements a
controlled mechanism for managing a Solana vote account with enhanced security and
support features.


## Location

The Vote Account Manager program is published at the following address:

`vamp2RmRDCj5ZW8rGzU1vKh3NCudJcbW8M8ey7q1CXn`


## Interacting with the Vote Account Manager

A script `vamp` is provided in the scripts directory.  It requires the installation of the
[solxact](https://github.com/bji/solxact)  program before it can be used.

vamp supports all available interactions with the Vote Account Manager program, and additionally
allows for the query for configuration state of a given vote account.  For more help:

```
$ vamp help

vamp is a utility script that can be used to interact with the Vote Account
Manager program.  All functionality of the program can be exercised using this
script.  Note that the 'solxact' program must be installed to use vamp.


Usage: vamp enter                      -- To start using VAMP
       vamp set-leave-epoch            -- To set a leave epoch
       vamp leave                      -- To stop using VAMP
       vamp set-administrator          -- To set the administrator
       vamp set-operational-authority  -- To set the operational authority
       vamp set-rewards-authority      -- To set the rewards authority
       vamp set-vote-authority         -- To set the vote authority
       vamp set-validator-identity     -- To set the validator identity
       vamp withdraw                   -- To withdraw from the vote account
       vamp set-commission             -- To set commission
       vamp show                       -- To show managed state
       vamp help                       -- To print this help message


For help on a specific command, use 'vamp help <COMMAND>', for example:

$ vamp help enter
```

## Using


The Vote Account Manager program can be used to manage a Solana vote account with better security and with
additional features, when compared with stock Vote Account management.

Specifically, the Vote Account Manager supports five authorities which control different aspects of the Vote
Account, compared to two highly overlapped authorities with stock Vote Account Management.

To understand how the Vote Account Manager program works, an understanding of how stock Vote Account management
works is required.

The stock Vote Account program has two authorities:

1. The vote authority.  This authority can do two things:
   1. Change the vote authority to a new vote authority
   2. Vote

2. The withdraw authority.  This authority can do five things:
   1. Change the vote authority to a new vote authority
   2. Change the withdraw authority to a new withdraw authority
   3. Change the validator identity
   4. Change the vote account commission
   5. Withdraw lamports from the vote account

The withdraw authority is thus a "master authority" that can do everything except vote.  This means that
anyone who has the withdraw authority has full control over the vote account and in effect "owns" it.
This makes the withdraw authority an incredibly important authority for a vote account - it is the "keys
to the castle".

This means that ideally the withdraw authority would be kept in very secure "cold storage", inaccessible
from the internet and protected from unauthorized access using the most secure methods available.  But
certain common operations also require the withdraw authority, thus requiring that the vote account owner
regularly access the keys to the withdraw authority account.  This makes it very difficult to actually
keep the vote authority as secure as it should be, since it must be kept in a form that can be regularly
accessed.


#### *Security*

The Vote Account Manager program improves the security situation by providing four authorities, each with a
clearly defined purpose:

1. The withdraw authority.  This authority can do two things:
   1. Change the administrator to a new administrator
   2. Remove the vote account from the program, returning it to the state it was in before the
      Vote Account Manager program was configured to manage the account

2. The administrator.  This authority can do two things:
   1. Change the operational authority to a new operational authority
   2. Change the rewards authority to a new rewards authority

3. The operational authority.  This authority can do two things:

   1. Change the vote authority to a new vote authority
   2. Change the validator identity

4. The rewards authority.  This authority can do two things:

   1. Change the vote account commission
   2. Withdraw lamports from the vote account

In addition, the normal vote authority of the vote account remains available and continues to have its
normal functionality, exercised via the stock Vote Account program:

5. The vote authority.  This authority can do two things:
   1. Change the vote authority to a new vote authority
   2. Vote

Because the withdraw authority has only two functions, which are expected to rarely if ever be needed,
the withdraw authority can be kept in secure cold storage.  It would really only be needed if the
administrator keypair were lost or compromised, in order to set a new administrator.  That is expected
to be an exceedingly rare situation, so keeping the withdraw authority in a highly inaccessible location
should be acceptable, which gives the maximum security appropriate for managing the "keys to the castle".

The administrator is a very important authority because it has effective control over all aspects of the
vote account; but because the administrator can be replaced by the withdraw authority, it can be stored in
a slightly less secure manner.  It too is expected to rarely be needed, since its purpose would only be
to replace the operational or rewards authorities if they are lost or compromised.  The administrator
can be kept in highly secure but still accessible storage; and the vote account owner, the only one
with the withdraw authority keypair, can assign full control over the vote account to an operator who
has the administrator keypair, knowing that this control can be revoked and given to a new administrator
any time this is necessary.

The operational authority is not expected to be used very regularly; its functions are rarely needed for
a vote account operator.  But it is possible that there could be a need to change the vote authority or
validator identity, so there may be management schemes which require that the operational authority be
kept "fairly hot".  The design of the Vote Account Manager makes this a reasonable operational strategy
since any compromise of the operational authority can be quickly taken care of by the administrator who
can set a new operational authority.

The rewards authority is expected to be needed on a very regular basis, because vote account rewards
typically will be withdrawn very regularly.  In addition, commission changes are not common, but are
more common than any other vote account operation besides withdrawing lamports.  Both of these functions
are controlled by the rewards authority, which can be kept very hot on a system that automates withdraw
of lamports as they accrue in the vote account.  Because the keypair for the rewards authority is kept
very "hot", it's the most likely to be compromised.  But if this happens, the administrator can change
to a new rewards authority.


#### *Commission Caps*

The owner of a vote account may optionally configure the Vote Account Management program to enforce
commission caps on the vote account.  There are two caps which can be set:

1. Max Commission - VAMP will not allow the commission for the vote account to be raised above this value
2. Max Commission Increase Per Epoch - VAMP will not allow commission to increase more than this percent
per epoch.  Decreases of any amount are allowed.

The use of this feature is entirely optional - at the time that the vote account is put under VAMP control,
whether or not to enforce commission caps, and the max commission and max commission increase that would
be enforced, is configured.

If commission caps are not enabled for a vote account, then the operator will not be restricted in how they
can set commission.  If commission caps are enabled for a vote account, then the operator will be restricted
by those caps.

If commission caps are enabled, then there is an additional restriction put on the vote account: it cannot
be removed from management by the Vote Account Manager program until a "leave epoch" has been set.  A
"leave epoch" can be set only by the withdraw authority (as a step in removing the vote account from
management by the program), must be set to an epoch at least 2 beyond the current epoch, and once set,
prevents any commission changes on the vote account from ever occuring again.  The purpose of this is to
ensure that stakers have an opportunity to respond to the operator's intention to remove the vote account
from the program; once removed, the operator can set commission without restriction, so requiring a minimum
one epoch delay in doing so via the "leave epoch" mechanism, stakers can be assured that they will always
have at least one epoch to act once the operator has indicated their intention to remove commission
restrictions on the vote account by removing it from Vote Account Manager program control.


## Typical Use Cases

Because the Vote Account Manager authorities can be configured in different ways, there are several typical
use cases supported.


1. **"Old style" management** with withdraw authority in cold storage.

In this use case, the operator wants to continue using their vote account as it had been traditionally used, with a
single authority controlling everything - but with the additional benefit of having a separate authority which can
re-assign that single authority if needed.  This mode of operation works nearly identically to the old way, but with
the added benefit of a super-secure cold storage keypair being used to "recover" a vote account which has been lost
due to its keys being compromised.

To accomplish this, use VAMP in its most basic mode: a single administrator keypair is assigned as
administrator, operational authority, and rewards authority.  Then the administrator has rights to
all functions of the vote account (except voting), but a new administrator can be chosed by the
withdraw authority at any time.

The command for setting a vote account up for management in this way:

```
$ vamp enter withdraw_authority.json vote_account.json administrator.json
```

After this command has been executed, the withdraw authority keypair can be put into deep cold storage, and
all further vote account operations will be performed using the administrator keypair.


2. **"Pragmatic" management**, without a separate operational authority.  In this case, the operator has recognized
that the functions of the operational authority - setting the vote authority and validator identity - are so
rare that it would be acceptable to keep them under control of the administrator.  But a separate rewards
authority is still used because those operations are expected to be performed regularly.  The commands for
setting a vote account up for management in this way:

```
$ vamp enter withdraw_authority.json vote_account.json administrator.json
$ vamp set-rewards-authority administrator.json vote_account.json rewards_authority.json
```

After these commands have been executed, the withdraw authority keypair is put into deep cold storage, the
administrator keypair is put into secure storage, expected to be very rarely if ever accessed, and the rewards
authority is put into fairly hot storage, in a place where regular withdraws from the vote account can be
done.


3. **"Full" management**, with all authorities separate.  This would be used in case the operator has some reason
to expect to need to change the vote authority or validator identity in future.  The commands for setting
a vote account up for management in this way:

```
$ vamp enter withdraw_authority.json vote_account.json administrator.json
$ vamp set-operational-authority administrator.json vote_account.json operational_authority.json
$ vamp set-rewards-authority administrator.json vote_account.json rewards_authority.json
```

After these commands have been executed, the withdraw authority keypair is put into deep cold storage, the
administrator keypair is put into secure storage, expected to be very rarely if ever accessed, the operational
authority is put wherever it is needed in order to accomplish the changes in vote authority and/or validator identity,
and the rewards authority is put into fairly hot storage, in a place where regular withdraws from the vote account can
be done.

4. **"With commission caps" management**, where any of the previous use cases are combined with commission
caps enforced by the Vote Account Manager program.  This is not necessary, but for those validator operators
who find value in giving stakers the peace of mind of knowing that commission cannot be raised beyond a
"promised" maximum commission, or changed more than a certain percent per epoch, these caps can be used.
Commission caps are set at the time that the vote account is put under Vote Account Manager control.  So
for example, to set a maximum commission of 10% with a maximum increase per epoch of 3%, the following
command would be used in place of the `vamp enter` command used in prior use case examples:

```
$ vamp enter withdraw_authority.json vote_account.json administrator.json 10 3
```


## Viewing Management Settings

To see the configured settings for a vote account managed by the Vote Account Manager program, use the
`vamp show` command, like so:

```
$ vamp show vote_account.json
```

The result will look like this:

```
Manager Account: ABsS4JPCWYyN1evPJpudm7apmEZp5NTocN3CAxKnSCQk
Withdraw Authority: 3cnbBcMULnSoyLtgGNwrEPdLiqwuzpU4bVpro2m71vn2
Administrator: 3wHoK6DTF9jPCqDQgp99RF88qo4QPyKca9gxxSMHYsMu
Operational Authority: B2YVSHfY3uK5egSzvt1unMchmdo3mxiC2grMxQpxf7DB
Rewards Authority: DchTjdEyR8ea46ofauxnVPMRZvBnCpkYkYixSXpQfNnk
Max Commission: 10
Max Commission Increase Per Epoch: 3
```

Alternately, if `json` is added to the end of the command, like so

```
$ vamp show vote_account.json json
```

The result will be json, which may be more easily parsed by scripts using the `jq` command.  For example:

```
$ vamp show vote_account.json json | jq .

{
  "manager_account_pubkey": "ABsS4JPCWYyN1evPJpudm7apmEZp5NTocN3CAxKnSCQk",
  "withdraw_authority": "3cnbBcMULnSoyLtgGNwrEPdLiqwuzpU4bVpro2m71vn2",
  "administrator": "3wHoK6DTF9jPCqDQgp99RF88qo4QPyKca9gxxSMHYsMu",
  "operational_authority": "B2YVSHfY3uK5egSzvt1unMchmdo3mxiC2grMxQpxf7DB",
  "rewards_authority": "DchTjdEyR8ea46ofauxnVPMRZvBnCpkYkYixSXpQfNnk",
  "max_commission": 10,
  "max_commission_increase_per_epoch": 3
}
```


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

   ```$ solana program show vamp2RmRDCj5ZW8rGzU1vKh3NCudJcbW8M8ey7q1CXn```

   The result must include the line:

    ```Authority: none```

    This line indicates that there is no one authorized to upgrade the program, making it
    non-upgradeable.

    You may also check the program on the solana explorer:

    [https://explorer.solana.com/address/vamp2RmRDCj5ZW8rGzU1vKh3NCudJcbW8M8ey7q1CXn](https://explorer.solana.com/address/vamp2RmRDCj5ZW8rGzU1vKh3NCudJcbW8M8ey7q1CXn)

    The following will indicate that the program is not upgradeable:

    ```Upgradeable         No```
   
2. Fetch the contents of the published on-chain Solana program:

   ```$ solana program dump vamp2RmRDCj5ZW8rGzU1vKh3NCudJcbW8M8ey7q1CXn vote-account-manager.so```

3. Compute the sha256sum of the Vote Account Manager program:

   ```$ sha256sum vote-account-manager.so```

4. The resulting SHA-256 checksum should be:

   `5cd8d716463defd58fed76cbf711100c753f41202a24c06816873343fd761a0e`

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

This will take a while to run to completion - about 10 minutes.  This is because it runs many
tests and it is conservative in waiting for commands to complete successfully before proceeding
to the next command.

You can inspect all of the tests that were run by looking at the files in the `test` directory.


## License

The Vote Account Manager program and all contents of this repository are Copyright 2022
by Shinobi Systems LLC, and licensed under the GNU Lesser General Public License Version 3.
