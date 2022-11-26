#!/bin/bash

set -e

# Usage: test.sh [<TEST_TO_RUN>...]
# If no tests are specified, all tests are run.

if [ -z "$SOURCE" ]; then
    echo "The SOURCE variable must be set to the root directory of the Shinobi Immortals source"
    exit 1
fi


TESTS=
while [ -n "$1" ]; do
    TESTS="$TESTS [$1]"
    shift
done


function should_run_test ()
{
    [ -z "$TESTS" ] || [[ "$TESTS" = *"[$1]"* ]]
}


function sanitize_output ()
{
    if [[ "$@" = "ERROR: {"* ]]; then
        JSON=`echo "$@" | cut -d ' ' -f 2- | sed "s/ Try 'solxact help' for help//g"`
        # Extract error
        if [ "`echo "$JSON" | jq .error`" != "null" ]; then
            if [ "`echo "$JSON" | jq .error.data`" != "null" -a                                                       \
                 "`echo "$JSON" | jq .error.data.err`" != "null" -a                                                   \
                 "`echo "$JSON" | jq .error.data.err.InstructionError[1]`" != "null" ]; then
                echo "$JSON" | jq -c .error.data.err.InstructionError[1]
                return
            fi
        fi
    elif [[ "$@" = "{"* ]]; then
        JSON="$@"
        if [ "`echo "$JSON" | jq .result`" != "null" ]; then
            if [ "`echo "$JSON" | jq .result.meta`" != "null" -a                                                      \
                 "`echo "$JSON" | jq .result.meta.err`" != "null" -a                                                  \
                 "`echo "$JSON" | jq .result.meta.err.InstructionError[1]`" != "null" ]; then
                echo "$JSON" | jq -c .result.meta.err.InstructionError[1]
                return
            fi
        fi
    fi
    
    # Just use the full output since it's not a parseable error
    echo "$@"
}


function current_epoch ()
{
    solana -u l epoch-info | grep ^Epoch: | cut -d ' ' -f 2
}


function sleep_until_epoch ()
{
    while true; do
        echo "Waiting until epoch $1"
        sleep 5
        CURRENT_EPOCH=`current_epoch`
        if [ $CURRENT_EPOCH == $1 ]; then
            # Just to be sure that the epoch is here, sleep once more
            sleep 5
            break
        fi
        echo "Waiting until epoch $1"
        sleep 5
    done
}

    
function sleep_until_next_epoch ()
{
    sleep_until_epoch $((`current_epoch`+1))
}


function assert ()
{
    RESULT=$?
    
    local TEST_NAME=$1
    shift
    local TEST_OUTPUT=`sanitize_output "$@"`

    if [ $RESULT -ne 0 ]; then
        echo "FAIL: $TEST_NAME failed when success was expected:"
        echo "$TEST_OUTPUT"
        echo
        exit 1
    else
        # Transactions can successfully be submitted (passing simulation) but then fail when actually run on chain;
        # so fetch the transaction signature and wait until its results are known
        if [[ "$TEST_OUTPUT" = Transaction\ signature:\ * ]]; then
            SIGNATURE=`echo "$TEST_OUTPUT" | cut -d ' ' -f 3`
            # Try for at most 10 seconds
            for i in `seq 1 10`; do
                JSON=`curl -s http://localhost:8899 -X POST -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"getTransaction\",\"params\":[\"$SIGNATURE\",\"json\"]}"`
                if [ -n "$JSON" ]; then
                    if [ "`echo $JSON | jq .result`" != "null" -a                                                     \
                         "`echo $JSON | jq .result.meta`" != "null" -a                                                \
                         "`echo $JSON | jq .result.meta.err`" != "null" ]; then
                        echo "FAIL: $TEST_NAME failed when success was expected:"
                        echo $JSON
                        exit 1
                    fi
                    echo "+ $TEST_NAME"
                    return
                fi
                sleep 1
            done
        fi
        echo "FAIL: $TEST_NAME failed when success was expected:"
        echo $TEST_OUTPUT
        exit 1
    fi
}


function assert_fail ()
{
    RESULT=$?
    
    local TEST_NAME=$1
    shift
    local EXPECTED_OUTPUT=$1
    shift
    local TEST_OUTPUT=`sanitize_output "$@"`

    if [ $RESULT -eq 0 ]; then
        # Transactions can successfully be submitted (passing simulation) but then fail when actually run on chain;
        # so fetch the transaction signature and wait until its results are known
        if [[ "$TEST_OUTPUT" = Transaction\ signature:\ * ]]; then
            SIGNATURE=`echo "$TEST_OUTPUT" | cut -d ' ' -f 3`
            # Try for at most 10 seconds
            for i in `seq 1 10`; do
                TEST_OUTPUT=`curl -s http://localhost:8899 -X POST -H "Content-Type: application/json" -d "{\"jsonrpc\":\"2.0\",\"id\":1,\"method\":\"getTransaction\",\"params\":[\"$SIGNATURE\",\"json\"]}"`
                if [ -n "$TEST_OUTPUT" ]; then
                    TEST_OUTPUT=`sanitize_output "$TEST_OUTPUT"`
                    break
                fi
                sleep 1
            done
            if [ -z "$TEST_OUTPUT" ]; then
                echo "FAIL: $TEST_NAME did not land ($SIGNATURE)"
                exit 1
            fi
        fi
    fi

    if [ "$EXPECTED_OUTPUT" != "$TEST_OUTPUT" ]; then
        echo "FAIL: $TEST_NAME incorrect output:"
        diff <(echo "$EXPECTED_OUTPUT") <(echo "$TEST_OUTPUT")
        echo
        exit 1
    else
        echo "- $TEST_NAME"
    fi
}


function make_funded_keypair ()
{
    local KEY_FILE=$1
    local SOL=$2

    if [ -n "$3" ]; then
        local COMMITMENT="--commitment finalized"
    else
        local COMMITMENT=
    fi

    echo "Creating keypair @ $KEY_FILE with $SOL SOL"

    solana-keygen new -o $KEY_FILE --no-bip39-passphrase >/dev/null 2>/dev/null

    if [ 0$SOL -gt 0 ]; then
        solana -u l airdrop -k $KEY_FILE $SOL $COMMITMENT >/dev/null 2>/dev/null
    fi
}


# Set up
if [ -z "$TEST_NO_SETUP" ]; then
    
    # Make a temporary directory to hold the validator ledger
    export LEDGER=`mktemp -d`
    
    # Start the test validator
    echo "Starting test validator @ $LEDGER"
    solana-test-validator --ledger $LEDGER --ticks-per-slot 16 --slots-per-epoch 100 >/dev/null 2>/dev/null &

    # Give it time to start
    echo "Waiting 10 seconds for it to settle"
    sleep 10

    # Create keys
    make_funded_keypair $LEDGER/program.json 0
    make_funded_keypair $LEDGER/withdrawer.json 100000
    make_funded_keypair $LEDGER/withdrawer2.json 100000
    make_funded_keypair $LEDGER/admin.json 100000
    make_funded_keypair $LEDGER/operations_authority.json 100000
    make_funded_keypair $LEDGER/rewards_authority.json 100000
    make_funded_keypair $LEDGER/vote_authority.json 100000
    make_funded_keypair $LEDGER/user.json 100
    make_funded_keypair $LEDGER/nonexistent_user.json 0
    make_funded_keypair $LEDGER/validator_identity.json 0
    make_funded_keypair $LEDGER/vote_account.json 0
    make_funded_keypair $LEDGER/validator_identity2.json 0
    make_funded_keypair $LEDGER/vote_account2.json 0
    make_funded_keypair $LEDGER/stake_account.json 0 finalized

    # Create a stake account
    echo "Creating stake account @ $LEDGER/stake_account.json"
    solana -u l create-stake-account -k $LEDGER/withdrawer.json $LEDGER/stake_account.json 1 >/dev/null 2>/dev/null

    # Create vote accounts
    echo "Creating vote accounts"
    solana -u l create-vote-account --fee-payer $LEDGER/withdrawer.json $LEDGER/vote_account.json                     \
           $LEDGER/validator_identity.json $LEDGER/withdrawer.json >/dev/null 2>/dev/null
    solana -u l create-vote-account --fee-payer $LEDGER/withdrawer2.json $LEDGER/vote_account2.json                   \
           $LEDGER/validator_identity2.json $LEDGER/withdrawer2.json --commitment=finalized >/dev/null 2>/dev/null
    solana -u l vote-update-commission -k $LEDGER/withdrawer.json $LEDGER/vote_account.json 0                         \
           $LEDGER/withdrawer.json >/dev/null 2>/dev/null
    solana -u l vote-update-commission -k $LEDGER/withdrawer2.json $LEDGER/vote_account2.json 0                       \
           $LEDGER/withdrawer2.json --commitment finalized >/dev/null 2>/dev/null

    # Build the program
    echo "Making build script"
    $SOURCE/make_build_program.sh $LEDGER/program.json > $LEDGER/build_program.sh
    chmod +x $LEDGER/build_program.sh
    echo "Building program"
    (cd $LEDGER;                                                                                                      \
     SDK_ROOT=~/.local/share/solana/install/active_release/bin/sdk SOURCE_ROOT=$SOURCE ./build_program.sh)
    
    # Deploy the program.
    echo "Deploying program"
    # Not sure why, but need to sleep to ensure the following succeeds
    sleep 1
    solana -k $LEDGER/withdrawer.json -u l program deploy --program-id $LEDGER/program.json $LEDGER/program.so        \
           --commitment finalized >/dev/null 2>/dev/null
else
    export LEDGER=`echo /tmp/tmp.*`
fi


# Set pubkey variables
export SELF_PROGRAM_PUBKEY=`solxact pubkey $LEDGER/program.json`
export SYSTEM_PROGRAM_PUBKEY=11111111111111111111111111111111
export VOTE_PROGRAM_PUBKEY=Vote111111111111111111111111111111111111111
export CLOCK_SYSVAR_PUBKEY=SysvarC1ock11111111111111111111111111111111
export MANAGER_ACCOUNT_PUBKEY=`solxact pda $SELF_PROGRAM_PUBKEY [ pubkey $LEDGER/vote_account.json ]                  \
                               | cut -d '.' -f 1`
export MANAGER_ACCOUNT2_PUBKEY=`solxact pda $SELF_PROGRAM_PUBKEY [ pubkey $LEDGER/vote_account2.json ]                \
                               | cut -d '.' -f 1`

# Set keypairs
export WITHDRAWER_KEYPAIR=$LEDGER/withdrawer.json
export WITHDRAWER2_KEYPAIR=$LEDGER/withdrawer2.json
export ADMIN_KEYPAIR=$LEDGER/admin.json
export OPERATIONS_AUTHORITY_KEYPAIR=$LEDGER/operations_authority.json
export REWARDS_AUTHORITY_KEYPAIR=$LEDGER/rewards_authority.json
export VOTE_AUTHORITY_KEYPAIR=$LEDGER/vote_authority.json
export USER_KEYPAIR=$LEDGER/user.json
export NONEXISTENT_USER_KEYPAIR=$LEDGER/nonexistent_user.json
export VALIDATOR_IDENTITY_KEYPAIR=$LEDGER/validator_identity.json
export VOTE_ACCOUNT_KEYPAIR=$LEDGER/vote_account.json
export VALIDATOR_IDENTITY2_KEYPAIR=$LEDGER/validator_identity2.json
export VOTE_ACCOUNT2_KEYPAIR=$LEDGER/vote_account2.json
export STAKE_ACCOUNT_KEYPAIR=$LEDGER/stake_account.json


source $SOURCE/test/test_enter

source $SOURCE/test/test_set_leave_epoch

source $SOURCE/test/test_leave

source $SOURCE/test/test_set_administrator

source $SOURCE/test/test_set_operational_authority

source $SOURCE/test/test_set_rewards_authority

source $SOURCE/test/test_set_vote_authority

source $SOURCE/test/test_set_validator_identity


# Tear down
#echo "Stopping test validator @ $LEDGER"
#
#solana-validator --ledger $LEDGER exit --force >/dev/null 2>/dev/null
#
## Wait until it's exited
#while ps auxww | grep solana-test-ledger | grep -v grep; do
#    sleep 1
#done
#
#rm -rf $LEDGER
