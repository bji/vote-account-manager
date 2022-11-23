
#include "solana_sdk.h"


// --------------------------------------------------------------------------------------------------------------------
// Public API (visible to clients) ------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------

// These are all of the instructions that this program can execute.  The enumerated value identifies the instruction
// to execute and is the first (and sometimes only) byte present in the instruction data.
typedef enum
{
    // "Enter" the program.  This puts a vote account withdraw authority under control of the program.  Only the
    // pre-existing vote acount withdraw authority may issue this instruction.
    //
    // # Account references
    //   0. `[WRITE]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[WRITE]` The Vote Account to enter into the program
    //   2. `[WRITE]` The account which will fund the creation of the Vote Account Manager state account
    //   3. `[SIGNER]` The current withdraw authority of the vote account
    //   4. `[]` The system program id
    //   5. `[]` The vote program id
    //   6. `[]` The clock sysvar id
    //
    // # Instruction data
    //   Instance of EnterInstructionData
    Instruction_Enter,                        = 0,

    // Set the leave epoch.  This is only necessary if commission caps are being enforced.  When the LeaveEpoch is
    // set, the validator will not be allowed to change commission while managed by this program ever again, and will
    // not be allowed to Leave the program until the leave epoch.  Only the original vote account withdraw authority
    // may issue this instruction.
    //
    // # Account references
    //   0. `[WRITE]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[]` The Vote Account
    //   2. `[SIGNER]` The withdraw authority of the Vote Account at the time that the program was Entered
    //
    // # Instruction data
    //   Instance of SetLeaveEpochInstructionData
    Instruction_SetLeaveEpoch                 = 1,

    // "Leaves" the program.  Returns the withdraw authority of the vote account to its original value (before
    // the program was Entered for this vote account), and deletes the vote account manager account associated with
    // the vote account and returns its lamports.  Only the original vote account withdraw authority may issue this
    // instruction.
    //
    // # Account references
    //   0. `[WRITE]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[WRITE]` The Vote Account
    //   2. `[SIGNER]` The withdraw authority of the Vote Account at the time that the program was Entered
    //   3. `[WRITE]` The account to receive the lamports stored in the Vote Account Manager state account
    //   4. `[]` The vote program id
    //   5. `[]` The clock sysvar id
    //
    // # Instruction data
    //   None
    Instruction_Leave,                        = 2,

    // Sets the pubkey of the administrator.  Only the original vote account withdraw authority may issue this
    // instruction.
    //
    // # Account references
    //   0. `[WRITE]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[]` The Vote Account
    //   2. `[SIGNER]` The withdraw authority of the Vote Account at the time that the program was Entered
    //
    // # Instruction data
    //   Instance of SetAuthorityInstructionData
    Instruction_SetAdministrator              = 3,

    // Sets the pubkey of the operational authority.  Only the administrator may issue this instruction.
    //
    // # Account references
    //   0. `[WRITE]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[]` The Vote Account
    //   2. `[SIGNER]` The administrator
    //
    // # Instruction data
    //   Instance of SetAuthorityInstructionData
    Instruction_SetOperationalAuthority       = 4,

    // Sets the pubkey of the rewards authority.  Only the administrator may issue this instruction.
    //
    // # Account references
    //   0. `[WRITE]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[]` The Vote Account
    //   2. `[SIGNER]` The administrator
    //
    // # Instruction data
    //   Instance of SetAuthorityInstructionData
    Instruction_SetRewardsAuthority           = 5,

    // Sets the vote authority of the vote account.  Only the operational authority may issue this instruction.
    //
    // # Account references
    //   0. `[]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[WRITE]` The Vote Account
    //   2. `[SIGNER]` The operational authority
    //   3. `[]` The vote program id
    //   4. `[]` The clock sysvar id
    //
    // # Instruction data
    //   Instance of SetAuthorityInstructionData
    Instruction_SetVoteAuthority              = 6,

    // Sets the validator identity of the vote account.  Only the operational authority may issue this instruction.
    //
    // # Account references
    //   0. `[]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[WRITE]` The Vote Account
    //   2. `[SIGNER]` The operational authority
    //   3. `[SIGNER]` New validator identity
    //   4. `[]` The vote program id
    //
    // # Instruction data
    //   None
    Instruction_SetValidatorIdentity          = 7,

    // Withdraws lamports from the vote account, but always leaves at least the rent exempt minimum in the vote
    // account.  Only the rewards authority may issue this instruction.
    //
    // # Account references
    //   0. `[]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[WRITE]` The Vote Account
    //   2. `[SIGNER]` The rewards authority
    //   3. `[WRITE]` The recipient account of the withdrawn lamports
    //   4. `[]` The vote program id
    //
    // # Instruction data
    //   Instance of WithdrawRewardsInstructionData
    Instruction_WithdrawRewards               = 8,

    // Sets the commission of the vote account.  Only the rewards authority may issue this instruction.
    //
    // # Account references
    //   0. `[]` Vote Account Manager state account, computed as the PDA of the vote account pubkey + bump seed
    //   1. `[WRITE]` The Vote Account
    //   2. `[SIGNER]` The rewards authority
    //   3. `[]` The vote program id
    //
    // # Instruction data
    //   Instance of SetCommissionInstructionData
    Instruction_SetCommission                 = 9

} Instruction;


// Data passed to an Enter instruction
typedef struct
{
    // First byte is the instruction index, which for Enter is 0
    uint8_t instruction_index;

    // The pubkey of the initial administrator, which is also initially the operational authority and rewards
    // authority
    SolPubkey admininstrator;

    // If this is true, then the max_commission and max_commission_change_rate values are used to control
    // commission changes by the validator.  If this is false, then the validator is not restricted by when and
    // how it can change commission by this program and max_commission and max_commission_change_rate are ignored.
    // Note that if this is true, then the Leave instruction cannot be executed until a LeaveEpoch has been set.
    bool use_commission_caps;

    // If use_commission_caps is true, then this gives the maximum commission value that will be allowed in the
    // SetCommission instruction for this vote account.
    uint8_t max_commission;

    // If use_commission_caps is true, then this gives the maximum change in commission that will be allowed per epoch
    // for this vote account.
    uint16_t max_commission_change_rate;

} EnterInstructionData;


// Data passed to a SetLeaveEpoch instruction
typedef struct
{
    // First byte is the instruction index, which for SetLeaveEpoch is 1
    uint8_t instruction_index;

    // The epoch in which the withdraw authority requests to be allowed to leave the program.  This must be
    // at least the current epoch + 2.
    uint64_t leave_epoch;

} SetLeaveEpochInstructionData;


// Data passed to a SetAdministrator, SetOperationalAuthority, SetRewardsAuthority, or SetVoteAuthority instruction
typedef struct
{
    // First byte is the instruction index, which for SetAdministrator is 3, for SetOperationalAuthority is 4,
    // for SetRewardsAuthority is 5, and for SetVoteAuthority is 6
    uint8_t instruction_index;

    // The pubkey of the new authority; which authority is actually set depends on which instruction has included this
    // data
    SolPubkey authority;

} SetAuthorityInstructionData;


// Data passed to a WithdrawRewards instruction
typedef struct
{
    // First byte is the instruction index, which for WithdrawRewards is 8
    uint8_t instruction_index;

    // Number of lamports to withdraw, or 0 to withdraw all available lamports down to the rent exempt minimum
    // balance of the vote account.
    uint64_t lamports;

} WithdrawRewardsInstructionData;


// Data passed to a SetCommission instruction
typedef struct
{
    // First byte is the instruction index, which for SetCommission is 9
    uint8_t instruction_index;

    // Commission value to set (0 - 100 inclusive)
    uint8_t commission;

} SetCommissionInstructionData;


// These are all custom errors that this program can return
typedef enum
{
    // Provided instruction data was invalid (didn't have an instruction_index)
    Error_InvalidData                         = 1000,

    // Size of the instruction data was not as expected by the instruction
    Error_InvalidDataSize                     = 1001,

    // Instruction data specified an invalid Instructon type
    Error_UnknownInstruction                  = 1002,

    // The number of accounts that were suppled in the instruction was incorrect for the instruction type
    Error_IncorrectNumberOfAccounts           = 1003,

    // An attempt to enter the vote manager program when the vote account is already being managed by the program
    Error_ManagerAccountAlreadyExists         = 1004,

    // Attempt to Enter for a vote account whose commission is already larger than the enforced maximum commission; or
    // attmept to set commission to a value larger than the enforced maximum commission
    Error_CommissionTooLarge                  = 1005,

    // An attempt to withdraw more lamports from the vote account than are available for withdraw
    Error_InsufficientLamports                = 1006,

    // The Clock sysvar could not be loaded.  This indicates a failure in the Solana blockchain.
    Error_FailedToGetClock                    = 1007,

    // Attempt to set the leave epoch or commission when leave epoch was already set
    Error_LeaveEpochAlreadySet                = 1008,

    // Attempt to set a leave epoch that is not at least one full epoch away
    Error_InvalidLeaveEpoch                   = 1009,

    // Attempt to set commission to a value that would exceed the allowed commission increase rate
    Error_CommissionChangeTooLarge            = 1010,

    // Errors Error_InvalidAccount_First through Error_InvalidAccount_Last are used to indicate an error in input
    // account, where the specific account that was faulty is the offset from Error_InvalidAccount_First
    Error_InvalidAccount_First                = 1100,
    Error_InvalidAccount_Last                 = 1199,

    // Errors Error_InvalidAccountPermissions_First through Error_InvalidAccountPermissions_Last are used to indicate
    // an error in input account permissions, where the specific account that was faulty is the offset from
    // Error_InvalidAccountPermissions_First
    Error_InvalidAccountPermissions_First     = 1200,
    Error_InvalidAccountPermissions_Last      = 1299,

    // Errors Error_InvalidData_First through Error_InvalidData_Last are used to indicate
    // an error in input data, where the field index of the specific data that was faulty is the offset from
    // Error_InvalidData_First
    Error_InvalidData_First                   = 1300,
    Error_InvalidData_Last                    = 1399,

} Error;


// This is the state stored in a vote account manager account
typedef struct
{
    // The withdraw authority at the time that the program was entered; this withdraw authority retains the ability to
    // set the administrator and to leave the program but possesses no other authority over the vote account while the
    // account is in the program
    SolPubkey original_withdraw_authority;

    // The administrator has rights to set the operational authority and rewards authority
    SolPubkey administrator;

    // The operational authority has rights to set the vote authority and validator identity
    SolPubkey operational_authority;

    // The rewards authority has rights to withdraw lamports from the vote account and set commission
    SolPubkey rewards_authority;

    // If this is true, then the max_commission and max_commission_increase_rate values are used to control commission
    // changes by the validator, and the validator will not be allowed to Leave the program until it has set a
    // LeaveEpoch, and until that LeaveEpoch.  If this is false, the no commission caps are enforced and the validator
    // can Leave the program at any time.
    bool use_commission_caps;

    // If use_commission_caps is true, then this gives the maximum commission value that will be allowed in the
    // SetCommission instruction for this vote account, otherwise this value is undefined.
    uint8_t max_commission;

    // If use_commission_caps is true, then this gives the maximum increase in commission that will be allowed per
    // epoch for this vote account, otherwise the meaning of this this value is undefined.
    uint8_t max_commission_increase_per_epoch;

    // If use_commission_caps is true, then this is epoch of the most recent commission change, otherwise the meaning
    // of this value is undefined.
    uint64_t commission_change_epoch;

    // If use_commission_caps is true, then this is the commission that was in effect in the epoch in which commission
    // was most recently changed, *before* any changes were made.  This allows commission to be changed multiple times
    // in an epoch without violating commission increase limits.  If use_commission_caps is false, then the meaning of
    // this value is undefined.
    uint8_t commission_change_epoch_original_commission;

    // If use_commission_caps is true, then the vote account cannot Leave this program until the leave epoch, which is
    // set by the SetLeaveEpoch instruction.  This is 0 before being set to a valid leave epoch.  If
    // use_commission_caps is false, then the meaning of this value is undefined.
    uint64_t leave_epoch;

} VoteAccountManagerState;


// --------------------------------------------------------------------------------------------------------------------
// Internal functions and macros used by public entrypoints
// --------------------------------------------------------------------------------------------------------------------

static uint64_t process_enter(const SolParameters *params, const SolSignerSeeds *singer_seeds);
static uint64_t process_set_leave_epoch(const SolParameters *params, const SolSignerSeeds *singer_seeds);
static uint64_t process_leave(const SolParameters *params, const SolSignerSeeds *singer_seeds);
static uint64_t process_set_administrator(const SolParameters *params, const SolSignerSeeds *singer_seeds);
static uint64_t process_set_operational_authority(const SolParameters *params, const SolSignerSeeds *singer_seeds);
static uint64_t process_set_rewards_authority(const SolParameters *params, const SolSignerSeeds *singer_seeds);
static uint64_t process_set_vote_authority(const SolParameters *params, const SolSignerSeeds *singer_seeds);
static uint64_t process_set_validator_identity(const SolParameters *params, const SolSignerSeeds *singer_seeds);
static uint64_t process_withdraw_rewards(const SolParameters *params, const SolSignerSeeds *singer_seeds);
static uint64_t process_set_commission(const SolParameters *params, const SolSignerSeeds *singer_seeds);

// Macro that computes the number of elements in a static array
#define ARRAY_LEN(a) (sizeof(a) / sizeof(*a))


// --------------------------------------------------------------------------------------------------------------------
// Public entrypoints (visible to the BPF runtime)
// --------------------------------------------------------------------------------------------------------------------

// Program entrypoint
uint64_t entrypoint(const uint8_t *input)
{
    SolParameters params;

    // At most 7 accounts are supported for any command.  This is enough for the Enter instruction, which has the
    // most accounts.
    SolAccountInfo account_info[7];
    params.ka = account_info;

    // Deserialize instruction parameters.
    if (!sol_deserialize(input, &params, ARRAY_LEN(account_info))) {
        return Error_InvalidData;
    }

    // If there isn't even an instruction code, the instruction is invalid.
    if (params.data_len < 1) {
        return Error_InvalidDataSize;
    }

    // The instruction code is the first byte in the instruction data
    uint8_t instruction_code = params.data[0];

    // All instructions include a manager_account as the first account, and a vote_account as the second account.
    if (params.ka_num < 2) {
        return Error_IncorrectNumberOfAccounts;
    }

    const SolAccountInfo *manager_account = &(params.ka[0]);
    const SolAccountInfo *vote_account = &(params.ka[1]);

    // The vote account must be a valid, existing vote account
    if ((vote_account->data_size == 0) || !SolPubkey_same(vote_account->owner, &(Constants.vote_program_pubkey))) {
        return Error_InvalidAccount_First + 1;
    }

    // All instructions require that the manager_account be the correct account for the given vote_account, and must
    // use signing seeds to sign transactions on behalf of this program.  Verify the accounts and compute the signing
    // seeds.

    // These values will hold the derived manager account address and bump seed
    SolPubkey pubkey;
    uint8_t bump_seed;

    // The program derived address seeds; the bump_seed will be derived
    SolSignerSeed seeds[] = { { vote_account->key, sizeof(SolPubkey) },
                              { &bump_seed, sizeof(bump_seed) } };

    // Only the first seed (vote account pubkey) is used when trying to find the address.  The second seed is the
    // bump_seed, which is filled in by this function call.
    uint64_t ret = sol_try_find_program_address(seeds, 1, &(Constants.self_program_pubkey), &pubkey, &bump_seed);
    if (ret) {
        return ret;
    }

    // Ensure that the program derived address that was computed is the same address that was passed in as the manager
    // account address
    if (!SolPubkey_same(&pubkey, /* manager account */ manager_account->key)) {
        return Error_InvalidAccount_First;
    }

    // If the instruction was Enter, then the manager account must either not exist, or must exist as owned by the
    // system program
    if (instruction_code == Instruction_Enter) {
        if ((manager_account->data_len > 0) &&
            !SolPubkey_same(manager_account->owner, &(Constants.system_program_pubkey))) {
            return Error_ManagerAccountAlreadyExists;
        }
    }
    // Else the manager account must be owned by this program, and exist with enough data to be big enough to hold an
    // instance of VoteAccountManagerState
    else if ((manager_account->data_len < sizeof(VoteAccountManagerState)) ||
             !SolPubkey_same(manager_account->owner, &(Constants.this_program_pubkey))) {
        return Error_InvalidAccount_First;
    }

    // Seeds to use when doing invoke_signed
    SolSignerSeeds signer_seeds = { seeds, ARRAY_LEN(seeds) };

    // For each instruction code, call the appropriate function to handle that instruction, and return its result
    switch (instruction_code) {
    case Instruction_Enter:
        return process_enter(&params, &signer_seeds);

    case Instruction_SetLeaveEpoch:
        return process_set_leave_epoch(&params, &signer_seeds);

    case Instruction_Leave:
        return process_leave(&params, &signer_seeds);

    case Instruction_SetAdministrator:
        return process_set_administrator(&params, &signer_seeds);

    case Instruction_SetOperationalAuthority:
        return process_set_operational_authority(&params, &signer_seeds);

    case Instruction_SetRewardsAuthority:
        return process_set_rewards_authority(&params, &signer_seeds);

    case Instruction_SetVoteAuthority:
        return process_set_vote_authority(&params, &signer_seeds);

    case Instruction_SetValidatorIdentity:
        return process_set_validator_identity(&params, &signer_seeds);

    case Instruction_WithdrawRewards:
        return process_withdraw_rewards(&params, &signer_seeds);

    case Instruction_SetCommission:
        return process_set_commission(&params, &signer_seeds);

    default:
        return Error_UnknownInstruction;
    }
}


// Provide a memcpy implementation; this allows structure assignments that the compiler will turn into memcpy which is
// safer than calling memcpy directly, because the compiler will always copy the correct number of bytes.
void *memcpy(void *dst, const void *src, int len)
{
    (void) sol_memcpy(dst, src, len);
    return dst;
}


// --------------------------------------------------------------------------------------------------------------------
// Private implementation details
// --------------------------------------------------------------------------------------------------------------------


// Function missing from solana_sdk.h
extern uint64_t sol_get_rent_sysvar(void *ret);


// Private structure definitions -------------------------------------------------------------------------------------

// These are constant values that the program can use.  An instance of _Constants is initialized by entrypoint and
// passed to all instruction entrypoints.
typedef struct
{
    // This constant ensures that data conforming to the "Solana Security.txt" format is present in the binary.
    // See: https://github.com/neodyme-labs/solana-security-txt
    const char security_txt[319];

    // This is the Vote Account Manager program pubkey.  It is the account address that actually stores this program.
    SolPubkey self_program_pubkey;

    // This is the system program pubkey
    SolPubkey system_program_pubkey;

    // This is the vote program pubkey
    SolPubkey vote_program_pubkey;

    // This is the clock sysvar pubkey
    SolPubkey clock_sysvar_pubkey;

} _Constants;


// These are identifiers of all "known accounts" which are accounts at fixed account addresses
typedef enum
{
    KnownAccount_NotKnown,               // Not a known account
    KnownAccount_SelfProgram,            // The vote-account-manager program itself
    KnownAccount_SystemProgram,          // The system program
    Knownaccount_VoteProgram,            // The vote program
    KnownAccount_ClockSysvar             // The clock sysvar

} KnownAccount;


// All indicators of account writability that may be checked against
typedef enum
{
    ReadOnly = 0,
    ReadWrite = 1

} AccountWriteable;


// All indicators of account signing that may be checked against
typedef enum
{
    NotSigner = 0,
    Signer = 1

} AccountSigner;


// Data structure stored in the Rent sysvar
typedef struct __attribute__((__packed__))
{
    uint64_t lamports_per_byte_year;

    uint8_t exemption_threshold[8];

    uint8_t burn_percent;

} Rent;


// bincode serialized.  This is only the needed fields.
typedef struct __attribute__((__packed__))
{
    SolPubkey node_pubkey;

    SolPubkey authorized_withdrawer;

    uint8_t commission;

    // The remaining fields are not presented here as they will never be referenced
} VoteStatePrefix;


// Data used in a System program transfer instruction
typedef struct __attribute__((__packed__))
{
    uint32_t instruction_code; // 2 for Transfer

    uint64_t amount;

} SystemTransferData;


// To be used as data to pass to the system program when invoking Allocate
typedef struct __attribute__((__packed__))
{
    uint32_t instruction_code; // 8 for Allocate

    uint64_t space;

} SystemAllocateData;


// To be used as data to pass to the system program when invoking Assign
typedef struct __attribute__((__packed__))
{
    uint32_t instruction_code; // 1 for Assign

    SolPubkey owner;

} SystemAssignData;


// bincode serialized
typedef struct __attribute__((__packed__))
{
    uint32_t enum_index; // 1 for Authorize

    SolPubkey pubkey;

    uint32_t authorize; // 0 for voter, 1 for withdrawer

} VoteAuthorizeData;


// bincode serialized
typedef struct __attribute__((__packed__))
{
    uint32_t enum_index; // 3 for Withdraw

    uint64_t lamports;

} VoteWithdrawData;


// bincode serialized
typedef struct __attribute__((__packed__))
{
    uint32_t enum_index; // 5 for UpdateCommission

    uint8_t lamports;

} VoteSetCommissionData;


// Constant values ----------------------------------------------------------------------------------------------------

// The _ARRAY values are provided at compile time on the command line
static const _Constants Constants =
{
    // security_txt
    "=======BEGIN SECURITY.TXT V1=======\0"
    "name\0Vote Account Manager\0"
    "project_url\0https://github.com/bji/vote-account-manager\0"
    "contacts\0email:shinobisystems@yahoo.com\0"
    "policy\0https://github.com/bji/vote-account-manager/security_policy.txt\0"
    "source_code\0https://github.com/bji/vote-account-manager\0"
    "=======END SECURITY.TXT V1=======\0",

    // Vote Account Manager program_pubkey
    SELF_PROGRAM_PUBKEY_ARRAY,

    // system_program_pubkey
    SYSTEM_PROGRAM_PUBKEY_ARRAY,

    // vote_program_pubkey
    VOTE_PROGRAM_PUBKEY_ARRAY,

    // clock_sysvar_pubkey
    CLOCK_SYSVAR_PUBKEY_ARRAY
};


// solana_sdk misses "const" in many places, so de-const to avoid compiler warnings.  The Constants instance is in a
// read-only data section and instructions which modify it actually have no effect.
#define Constants (* ((_Constants *) &Constants))


// Account helpers ----------------------------------------------------------------------------------------------------

// These helper macros make it easier and less error-prone to enforce that specific accounts are present in the
// instruction accounts list

#define DECLARE_ACCOUNTS uint8_t _account_num = 0;

#define DECLARE_ACCOUNT(n, name, writable, signer, known_account) }                                                    \
    if (_account_num == params->ka_num) {                                                                              \
        return Error_IncorrectNumberOfAccounts;                                                                        \
    }                                                                                                                  \
    SolAccountInfo *name = &(params->ka[_account_num++]);                                                              \
    if (!check_known_account(name, (known_account))) {                                                                 \
        return Error_InvalidAccount_First + (_account_num - 1);                                                        \
    }                                                                                                                  \
    if (((writable == ReadWrite) && !name->is_writable) ||                                                             \
        ((signer == Signer) && !name->is_signer)) {                                                                    \
        return Error_InvalidAccountPermissions_First + (_account_num - 1);                                             \
    } {

#define DECLARE_ACCOUNTS_NUMBER(n) if (params->ka_num != (n)) { return Error_IncorrectNumberOfAccounts; }


// This function implements checking an input account to ensure that it is the same account as the specified account,
// unless the specified account is KnownAccount_NotKnown
static bool check_known_account(const SolAccountInfo *account, KnownAccount known_account)
{
    const SolPubkey *known_pubkey;

    switch (known_account) {
    case KnownAccount_NotKnown:
        return true;

    case KnownAccount_SelfProgram:
        known_pubkey = &(Constants.self_program_pubkey);
        break;

    case KnownAccount_SystemProgram:
        known_pubkey = &(Constants.system_program_pubkey);
        break;

    case KnownAccount_VoteProgram:
        known_pubkey = &(Constants.vote_program_pubkey);
        break;

    case KnownAccount_ClockSysvar:
        known_pubkey = &(Constants.clock_sysvar_pubkey);
        break;
    }

    return SolPubkey_same(account->key, known_pubkey);
}


// Computes rent exempt minimum lamports for a given account size.  This may overestimate slightly.
static uint64_t get_rent_exempt_minimum(uint64_t account_size)
{
    // Get the rent sysvar value
    Rent rent;
    uint64_t result = sol_get_rent_sysvar(&rent);

    // Unfortunately the exemption threshold is in f64 format.  This makes it super difficult to work with since BPF
    // doesn't have floating point instructions.  So do manual computation using the bits of the floating point value.
    uint64_t u = * (uint64_t *) rent.exemption_threshold;
    uint64_t exp = ((u >> 52) & 0x7FF);

    if (result || // sol_get_rent_sysvar failed, so u and exp are bogus
        (u & 0x8000000000000000ul) || // negative exemption_threshold
        ((exp == 0) || (exp == 0x7FF))) { // subnormal values
        // Unsupported and basically nonsensical rent exemption threshold.  Just use some hopefully sane default based
        // on historical values that were true for 2021/2022: lamports_per_byte_year = 3480, exemption_threshold = 2
        // years
        return (account_size + 128) * 3480 * 2;
    }

    // 128 bytes are added for account overhead
    uint64_t min = (account_size + 128) * rent.lamports_per_byte_year;

    if (exp >= 1023) {
        min *= (1 << (exp - 1023));
    }
    else {
        min /= (1 << (1023 - exp));
    }

    // Reduce fraction to 10 bits, to avoid overflow.  Keep track of whether or not to round up.
    uint64_t fraction = u & 0x000FFFFFFFFFFFFFul;
    bool round_up = (fraction & 0x3FFFFFFFFFFul);

    fraction >>= 42;
    if (round_up) {
        fraction += 1;
    }

    fraction *= min;
    fraction /= 0x3FF;

    return min + fraction;
}


// Returns the current commission of a vote account in *commission_return; and returns true on success and false on
// failure (due to a bogus vote account)
static bool get_vote_account_commission(const SolAccountInfo *vote_account, uint8_t *commission_return)
{
    if (vote_account->data_len < sizeof(VoteStatePrefix)) {
        return false;
    }

    *commission_return = ((const VoteStatePrefix *) vote_account->data)->commission;

    return true;
}


// Instruction processing ---------------------------------------------------------------------------------------------

// Processes an Enter instruction.  Note that entrypoint already guaranteed that the manager_account doesn't exist as
// a manager account yet, and that vote_account has data and is owned by the vote program, and that manager_account is
// the correct Vote Account Manager state account for vote_account.
static uint64_t process_enter(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   funding_account,               ReadWrite,  Signer,     KnownAccount_NotKnown);
        DECLARE_ACCOUNT(3,   withdraw_authority,            ReadOnly,   Signer,     KnownAccount_NotKnown);
        DECLARE_ACCOUNT(4,   system_program_id,             ReadOnly,   NotSigner,  KnownAccount_SystemProgram);
        DECLARE_ACCOUNT(5,   vote_program_id,               ReadOnly,   NotSigner,  KnownAccount_VoteProgram);
        DECLARE_ACCOUNT(6,   clock_sysvar,                  ReadOnly,   NotSigner,  KnownAccount_ClockSysvar);
    }
    DECLARE_ACCOUNTS_NUMBER(7);

    // Ensure that the instruction data is the correct size
    if (params->data_len != sizeof(EnterInstructionData)) {
        return Error_InvalidDataSize;
    }

    // Cast instruction data
    const EnterInstructionData *instruction_data = (const EnterInstructionData *) params->data;

    // Enforce validity of instruction data
    if (instruction_data->use_commission_caps) {
        // Max commission > 100 is nonsensical
        if (instruction_data->max_commission > 100) {
            return Error_InvalidData_First + 5;
        }
        // Max commission change rate > 100 is nonsensical
        if (instruction_data->max_commission_change_rate > 100) {
            return Error_InvalidData_First + 6;
        }

        // Check to make sure that the current commission is not already larger than the max_commission
        uint8_t commission;
        if (!get_vote_account_commission(vote_account, &commission)) {
            return Error_InvalidAccount_First + 1;
        }

        if (commission > instruction_data->max_commission) {
            return Error_CommissionTooLarge;
        }
    }

    // Get rent exempt minimum lamports needed for the manager account
    uint64_t rent_exempt_minimum = get_rent_exempt_minimum(sizeof(VoteAccountManagerState));

    // Fund the manager account
    if (*(manager_account->lamports) < rent_exempt_minimum) {
        uint64_t lamports = rent_exempt_minimum - *(manager_account->lamports);

        SolInstruction instruction;

        instruction.program_id = &(Constants.system_program_pubkey);

        SolAccountMeta account_metas[] =
              ///   0. `[writable, signer]` The source account.
            { { /* pubkey */ funding_account->key, /* is_writable */ true, /* is_signer */ true },
              ///   1. `[writable]` The destination account.
              { /* pubkey */ manager_account->key, /* is_writable */ true, /* is_signer */ false } };

        instruction.accounts = account_metas;
        instruction.account_len = ARRAY_LEN(account_metas);

        SystemTransferData data = { 2, lamports };

        instruction.data = (uint8_t *) &data;
        instruction.data_len = sizeof(data);

        ret = sol_invoke(&instruction, params->ka, params->ka_num);
        if (ret) {
            return ret;
        }
    }

    // Allocate space for the account
    if (manager_account->data_len < sizeof(VoteAccountManagerState)) {
        // If the account is owned by the system program, then use the system Alloc instruction to allocate space
        if (SolPubkey_same(manager_account->owner, &(Constants.system_program_pubkey))) {
            SolInstruction instruction;

            instruction.program_id = &(Constants.system_program_pubkey);

            SolAccountMeta account_metas[] =
                  ///   0. `[WRITE, SIGNER]` New account
                { { /* pubkey */ manager_account->key, /* is_writable */ true, /* is_signer */ true } };

            instruction.accounts = account_metas;
            instruction.account_len = ARRAY_LEN(account_metas);

            SystemAllocateData data = { 8, space };

            instruction.data = (uint8_t *) &data;
            instruction.data_len = sizeof(data);

            uint64_t ret = sol_invoke_signed(&instruction, params->ka, params->ka_num, signer_seeds, 1);
            if (ret) {
                return ret;
            }
        }
        // Else the account must be owned by this program (otherwise the test for account existence above would have
        // failed), so realloc the data segment so that the newly sized data segment is available.  Setting the 64 bit
        // value immediately preceeding the account is how this is accomplished.
        else {
            ((uint64_t *) (manager_account->data))[-1] = size;

            manager_account->data_len = size;
        }
    }

    // Assign the manager account ownership
    if (!SolPubkey_same(manager_account->owner, &(Constants.self_program_pubkey))) {
        SolInstruction instruction;

        instruction.program_id = &(Constants.system_program_pubkey);

        // This will only succeed if the existing owner is the system program; if it's any other program, this
        // will fail
        SolAccountMeta account_metas[] =
              ///   0. `[WRITE, SIGNER]` Assigned account public key
            { { /* pubkey */ manager_account->key, /* is_writable */ true, /* is_signer */ true } };

        instruction.accounts = account_metas;
        instruction.account_len = ARRAY_LEN(account_metas);

        SystemAssignData data = { 1, *(manager_account->key) };

        instruction.data = (uint8_t *) &data;
        instruction.data_len = sizeof(data);

        uint64_t ret = sol_invoke_signed(&instruction, params->ka, params->ka_num, signer_seeds, 1);
        if (ret) {
            return ret;
        }
    }

    // Use the vote program to set the vote account withdraw authority to the manager account, which then allows
    // only this program to sign as withdraw authority on instructions for the vote account
    SolInstruction instruction;

    instruction.program_id = &(Constants.vote_program_pubkey);

    SolAccountMeta account_metas[] =
          ///   0. `[WRITE]` Vote account to be updated with the Pubkey for authorization
        { { /* pubkey */ vote_account->key, /* is_writable */ true, /* is_signer */ false },
          ///   1. `[]` Clock sysvar
          { /* pubkey */ &(Constants.clock_sysvar_pubkey), /* is_writable */ false, /* is_signer */ false },
          ///   2. `[SIGNER]` Vote or withdraw authority
          { /* pubkey */ withdraw_authority->key, /* is_writable */ false, /* is_signer */ true } };

    instruction.accounts = account_metas;
    instruction.account_len = ARRAY_LEN(account_metas);

    VoteAuthorizeData data = { 1, *(manager_account->key), 1 };

    instruction.data = (uint8_t *) &data;
    instruction.data_len = sizeof(data);

    ret = sol_invoke(&instruction, params->ka, params->ka_num);
    if (ret) {
        return ret;
    }

    // Save the manager account state
    (* (VoteAccountManagerState *) manager_account->data) = {
        *(withdraw_authority->key),
        *(instruction_data->administrator),
        *(instruction_data->administrator),
        *(instruction_data->administrator),
        instruction_data->use_commission_caps,
        instruction_data->max_commission,
        instruction_data->max_commission_increase_per_epoch,
        0, // commission_change_epoch
        0, // commission_change_epoch_original_commission
        0  // leave_epoch
    };

    return 0;
}


// Processes a SetLeaveEpoch instruction.  Note that entrypoint already guaranteed that the manager_account exists as
// a manager account already, and that vote_account has data and is owned by the vote program, and that
// manager_account is the correct Vote Account Manager state account for vote_account.
static uint64_t process_set_leave_epoch(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadOnly,   NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   withdraw_authority,            ReadOnly,   Signer,     KnownAccount_NotKnown);
    }
    DECLARE_ACCOUNTS_NUMBER(2);

    // This is the vote account manager state
    const VoteAccountManagerState *manager_account_state = (const VoteAccountManagerState *) manager_account->data;

    // Ensure that the provided withdraw authority is the withdraw authority that was saved in the manager account
    if (!SolPubkey_same(&(manager_account_state->original_withdraw_authority), withdraw_authority->key)) {
        return Error_InvalidAccount_First + 2;
    }

    // If the leave epoch is already set, then cannot re-set it
    if (manager_account_state->leave_epoch > 0) {
        return Error_LeaveEpochAlreadySet;
    }

    // Ensure that the instruction data is the correct size
    if (params->data_len != sizeof(SetLeaveEpochInstructionData)) {
        return Error_InvalidDataSize;
    }

    // Cast instruction data
    const SetLeaveEpochInstructionData *instruction_data = (const SetLeaveEpochInstructionData *) params->data;

    // Ensure that the leave epoch is at least 1 full epoch beyond the current epoch
    Clock clock;
    if (sol_get_clock_sysvar(&clock)) {
        return Error_FailedToGetClock;
    }
    if (instruction_data->leave_epoch < (clock->epoch + 2)) {
        return Error_InvalidLeaveEpoch;
    }

    // Set the leave epoch
    manager_account_state->leave_epoch = instruction_data->leave_epoch;

    return 0;
}


// Processes a Leave instruction.  Note that entrypoint already guaranteed that the manager_account exists as a
// manager account already, and that vote_account has data and is owned by the vote program, and that manager_account
// is the correct Vote Account Manager state account for vote_account.
static uint64_t process_leave(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   withdraw_authority,            ReadOnly,   Signer,     KnownAccount_NotKnown);
        DECLARE_ACCOUNT(3,   recipient_account,             ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(4,   vote_program_id,               ReadOnly,   NotSigner,  KnownAccount_VoteProgram);
        DECLARE_ACCOUNT(5,   clock_sysvar,                  ReadOnly,   NotSigner,  KnownAccount_ClockSysvar);
    }
    DECLARE_ACCOUNTS_NUMBER(6);

    // This is the vote account manager state
    const VoteAccountManagerState *manager_account_state = (const VoteAccountManagerState *) manager_account->data;

    // Ensure that the provided withdraw authority is the withdraw authority that was saved in the manager account
    if (!SolPubkey_same(&(manager_account_state->original_withdraw_authority), withdraw_authority->key)) {
        return Error_InvalidAccount_First + 3;
    }

    // If commission change limits are in effect, then check to make sure that the LeaveEpoch has been set and that
    // the current epoch is at least the LeaveEpoch.  This gives stakers an epoch to take action should they want to.
    if (manager_account_state->use_commission_caps) {
        if (manager_account_state->leave_epoch == 0) {
            return Error_LeaveEpochNotSet;
        }

        Clock clock;
        if (sol_get_clock_sysvar(&clock)) {
            return Error_FailedToGetClock;
        }

        if (clock->epoch < manager_account_state->leave_epoch) {
            return Error_CannotLeaveYet;
        }
    }

    // Use the vote program to set the vote account withdraw authority to its original value.
    SolInstruction instruction;

    instruction.program_id = &(Constants.vote_program_pubkey);

    SolAccountMeta account_metas[] =
          ///   0. `[WRITE]` Vote account to be updated with the Pubkey for authorization
        { { /* pubkey */ vote_account->key, /* is_writable */ true, /* is_signer */ false },
          ///   1. `[]` Clock sysvar
          { /* pubkey */ &(Constants.clock_sysvar_pubkey), /* is_writable */ false, /* is_signer */ false },
          ///   2. `[SIGNER]` Vote or withdraw authority
          { /* pubkey */ manager_account->key, /* is_writable */ false, /* is_signer */ true } };

    instruction.accounts = account_metas;
    instruction.account_len = ARRAY_LEN(account_metas);

    VoteAuthorizeData data = { 1, *(manager_account->key), 1 };

    instruction.data = (uint8_t *) &data;
    instruction.data_len = sizeof(data);

    ret = sol_invoke_signed(&instruction, params->ka, params->ka_num, signer_seeds, 1);
    if (ret) {
        return ret;
    }

    // Now return all lamports from the manager account to the recipient account, thus deleting the manager acount
    *(recipient_account->lamports) += *(manager_account->lamports);
    *(manager_account->lamports) = 0;

    return 0;
}


// Processes a SetAdministrator instruction.  Note that entrypoint already guaranteed that the manager_account exists
// as a manager account already, and that vote_account has data and is owned by the vote program, and that
// manager_account is the correct Vote Account Manager state account for vote_account.
static uint64_t process_set_administrator(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadOnly,   NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   withdraw_authority,            ReadOnly,   Signer,     KnownAccount_NotKnown);
    }
    DECLARE_ACCOUNTS_NUMBER(3);

    // This is the vote account manager state
    const VoteAccountManagerState *manager_account_state = (const VoteAccountManagerState *) manager_account->data;

    // Ensure that the provided withdraw authority is the withdraw authority that was saved in the manager account
    if (!SolPubkey_same(&(manager_account_state->original_withdraw_authority), withdraw_authority->key)) {
        return Error_InvalidAccount_First + 2;
    }

    // Ensure that the instruction data is the correct size
    if (params->data_len != sizeof(SetAuthorityInstructionData)) {
        return Error_InvalidDataSize;
    }

    // Cast instruction data
    const SetAuthorityInstructionData *instruction_data = (const SetAuthorityInstructionData *) params->data;

    // Overwrite the administrator pubkey
    manager_account_state->administrator = instruction_data->authority;

    return 0;
}


// Processes a SetOperationalAuthority instruction.  Note that entrypoint already guaranteed that the manager_account
// exists as a manager account already, and that vote_account has data and is owned by the vote program, and that
// manager_account is the correct Vote Account Manager state account for vote_account.
static uint64_t process_set_operational_authority(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadOnly,   NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   administrator,                 ReadOnly,   Signer,     KnownAccount_NotKnown);
    }
    DECLARE_ACCOUNTS_NUMBER(3);

    // This is the vote account manager state
    const VoteAccountManagerState *manager_account_state = (const VoteAccountManagerState *) manager_account->data;

    // Ensure that the provided administrator is the administrator that was saved in the manager account
    if (!SolPubkey_same(&(manager_account_state->administrator), administrator->key)) {
        return Error_InvalidAccount_First + 2;
    }

    // Ensure that the instruction data is the correct size
    if (params->data_len != sizeof(SetAuthorityInstructionData)) {
        return Error_InvalidDataSize;
    }

    // Cast instruction data
    const SetAuthorityInstructionData *instruction_data = (const SetAuthorityInstructionData *) params->data;

    // Overwrite the operational authority pubkey
    manager_account_state->operational_authority = instruction_data->authority;

    return 0;
}


// Processes a SetRewardsAuthority instruction.  Note that entrypoint already guaranteed that the manager_account
// exists as a manager account already, and that vote_account has data and is owned by the vote program, and that
// manager_account is the correct Vote Account Manager state account for vote_account.
static uint64_t process_set_rewards_authority(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadOnly,   NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   administrator,                 ReadOnly,   Signer,     KnownAccount_NotKnown);
    }
    DECLARE_ACCOUNTS_NUMBER(3);

    // This is the vote account manager state
    const VoteAccountManagerState *manager_account_state = (const VoteAccountManagerState *) manager_account->data;

    // Ensure that the provided administrator is the administrator that was saved in the manager account
    if (!SolPubkey_same(&(manager_account_state->administrator), administrator->key)) {
        return Error_InvalidAccount_First + 2;
    }

    // Ensure that the instruction data is the correct size
    if (params->data_len != sizeof(SetAuthorityInstructionData)) {
        return Error_InvalidDataSize;
    }

    // Cast instruction data
    const SetAuthorityInstructionData *instruction_data = (const SetAuthorityInstructionData *) params->data;

    // Overwrite the rewards authority pubkey
    manager_account_state->rewards_authority = instruction_data->authority;

    return 0;
}


// Processes a SetVoteAuthority instruction.  Note that entrypoint already guaranteed that the manager_account exists
// as a manager account already, and that vote_account has data and is owned by the vote program, and that
// manager_account is the correct Vote Account Manager state account for vote_account.
static uint64_t process_set_vote_authority(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadOnly,   NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   operational_authority,         ReadOnly,   Signer,     KnownAccount_NotKnown);
        DECLARE_ACCOUNT(3,   vote_program_id,               ReadOnly,   NotSigner,  KnownAccount_VoteProgram);
        DECLARE_ACCOUNT(4,   clock_sysvar,                  ReadOnly,   NotSigner,  KnownAccount_ClockSysvar);
    }
    DECLARE_ACCOUNTS_NUMBER(5);

    // Ensure that the instruction data is the correct size
    if (params->data_len != sizeof(SetAuthorityInstructionData)) {
        return Error_InvalidDataSize;
    }

    // Cast instruction data
    const SetAuthorityInstructionData *instruction_data = (const SetAuthorityInstructionData *) params->data;

    // This is the vote account manager state
    const VoteAccountManagerState *manager_account_state = (const VoteAccountManagerState *) manager_account->data;

    // Ensure that the provided operational authority is the operational authority of the manager account
    if (!SolPubkey_same(&(manager_account_state->operational_authority), operational_authority->key)) {
        return Error_InvalidAccount_First + 3;
    }

    // Issue a vote Authorize instruction to authorize the voter
    SolInstruction instruction;

    instruction.program_id = &(Constants.vote_program_pubkey);

    SolAccountMeta account_metas[] =
          ///   0. `[WRITE]` Vote account to be updated with the Pubkey for authorization
        { { /* pubkey */ vote_account->key, /* is_writable */ true, /* is_signer */ false },
          ///   1. `[]` Clock sysvar
          { /* pubkey */ &(Constants.clock_sysvar_pubkey), /* is_writable */ false, /* is_signer */ false },
          ///   2. `[SIGNER]` Vote or withdraw authority
          { /* pubkey */ manager_account->key, /* is_writable */ false, /* is_signer */ true } };

    instruction.accounts = account_metas;
    instruction.account_len = ARRAY_LEN(account_metas);

    VoteAuthorizeData data = { 1, instruction_data->vote_authority, 0 };

    instruction.data = (uint8_t *) &data;
    instruction.data_len = sizeof(data);

    return sol_invoke_signed(&instruction, params->ka, params->ka_num, signer_seeds, 1);
}


// Processes a SetValidatorIdentity instruction.  Note that entrypoint already guaranteed that the manager_account
// exists as a manager account already, and that vote_account has data and is owned by the vote program, and that
// manager_account is the correct Vote Account Manager state account for vote_account.
static uint64_t process_set_validator_identity(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadOnly,   NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   operational_authority,         ReadOnly,   Signer,     KnownAccount_NotKnown);
        DECLARE_ACCOUNT(3,   new_validator_identity,        ReadOnly,   Signer,     KnownAccount_NotKnown);
        DECLARE_ACCOUNT(4,   vote_program_id,               ReadOnly,   NotSigner,  KnownAccount_VoteProgram);
    }
    DECLARE_ACCOUNTS_NUMBER(5);

    // Ensure that the instruction data is the correct size
    if (params->data_len != sizeof(WithdrawRewardsInstructionData)) {
        return Error_InvalidDataSize;
    }

    // Cast instruction data
    const SetValidatorIdentityInstructionData *instruction_data =
        (const SetValidatorIdentityInstructionData *) params->data;

    // This is the vote account manager state
    const VoteAccountManagerState *manager_account_state = (const VoteAccountManagerState *) manager_account->data;

    // Ensure that the provided operational authority is the operational authority of the manager account
    if (!SolPubkey_same(&(manager_account_state->operational_authority), operational_authority->key)) {
        return Error_InvalidAccount_First + 3;
    }

    // Issue a vote UpdateValidatorIdentity instruction to authorize the voter
    SolInstruction instruction;

    instruction.program_id = &(Constants.vote_program_pubkey);

    SolAccountMeta account_metas[] =
          ///   0. `[WRITE]` Vote account to be updated with the given authority public key
        { { /* pubkey */ vote_account->key, /* is_writable */ true, /* is_signer */ false },
          ///   1. `[SIGNER]` New validator identity (node_pubkey)
          { /* pubkey */ new_validator_identity->key, /* is_writable */ false, /* is_signer */ true },
          ///   2. `[SIGNER]` Withdraw authority
          { /* pubkey */ manager_account->key, /* is_writable */ false, /* is_signer */ true } };

    instruction.accounts = account_metas;
    instruction.account_len = ARRAY_LEN(account_metas);

    instruction.data = 0;
    instruction.data_len = 0;

    return sol_invoke_signed(&instruction, params->ka, params->ka_num, signer_seeds, 1);
}


// Processes a WithdrawRewards instruction.  Note that entrypoint already guaranteed that the manager_account exists
// as a manager account already, and that vote_account has data and is owned by the vote program, and that
// manager_account is the correct Vote Account Manager state account for vote_account.
static uint64_t process_withdraw_rewards(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadOnly,   NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   rewards_authority,             ReadOnly,   Signer,     KnownAccount_NotKnown);
        DECLARE_ACCOUNT(3,   recipient_account,             ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(4,   vote_program_id,               ReadOnly,   NotSigner,  KnownAccount_VoteProgram);
    }
    DECLARE_ACCOUNTS_NUMBER(5);

    // Ensure that the instruction data is the correct size
    if (params->data_len != sizeof(WithdrawRewardsInstructionData)) {
        return Error_InvalidDataSize;
    }

    // Cast instruction data
    const WithdrawRewardsInstructionData *instruction_data = (const WithdrawRewardsInstructionData *) params->data;

    // This is the vote account manager state
    const VoteAccountManagerState *manager_account_state = (const VoteAccountManagerState *) manager_account->data;

    // Ensure that the provided rewards authority is the rewards authority of the manager account
    if (!SolPubkey_same(&(manager_account_state->rewards_authority), rewards_authority->key)) {
        return Error_InvalidAccount_First + 3;
    }

    // Compute maximum lamports that may be withdrawn from the vote account
    uint64_t maximum_allowed_lamports = 0;
    uint64_t rent_exempt_minimum = get_rent_exempt_minimum(sizeof(VoteAccountManagerState));

    if (*(vote_account->lamports) > rent_exempt_minimum) {
        maximum_allowed_lamports = *(vote_account->lamports) - rent_exempt_minimum;
    }

    // Compute lamports to withdraw
    uint64_t lamports_to_withdraw;

    // If the number of lamports to withdraw was not specified, the maximum that can be withdrawn is used
    if (instruction_data->lamports == 0) {
        lamports_to_withdraw = maximum_allowed_lamports;
    }
    // Else if the requested lamports withdraw is too large, then return an error
    else if (instruction_data->lamports > maximum_allowed_lamports) {
        return Error_InsufficientLamports;
    }
    // Else, withdraw the requested lamports, since it is provided and valid
    else {
        lamports_to_withdraw = instruction_data->lamports;
    }

    // Issue a vote withdraw instruction to withdraw the lamports from the vote account
    SolInstruction instruction;

    instruction.program_id = &(Constants.vote_program_pubkey);

    SolAccountMeta account_metas[] =
          ///   0. `[WRITE]` Vote account to be updated with the Pubkey for authorization
        { { /* pubkey */ vote_account->key, /* is_writable */ true, /* is_signer */ false },
          ///   1. `[WRITE]` Recipient account
          { /* pubkey */ recipient_account->key, /* is_writable */ true, /* is_signer */ false },
          ///   2. `[SIGNER]` Withdraw authority
          { /* pubkey */ manager_account->key, /* is_writable */ false, /* is_signer */ true } };

    instruction.accounts = account_metas;
    instruction.account_len = ARRAY_LEN(account_metas);

    VoteWithdrawData data = { 3, lamports_to_withdraw };

    instruction.data = (uint8_t *) &data;
    instruction.data_len = sizeof(data);

    return sol_invoke_signed(&instruction, params->ka, params->ka_num, signer_seeds, 1);
}


// Processes a SetCommission instruction.  Note that entrypoint already guaranteed that the manager_account exists as
// a manager account already, and that vote_account has data and is owned by the vote program, and that
// manager_account is the correct Vote Account Manager state account for vote_account.
static uint64_t process_set_commission(const SolParameters *params, const SolSignerSeeds *singer_seeds)
{
    // Declare accounts, which checks the permissions of all accounts, and the identity of known accounts
    DECLARE_ACCOUNTS {
        DECLARE_ACCOUNT(0,   manager_account,               ReadOnly,   NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(1,   vote_account,                  ReadWrite,  NotSigner,  KnownAccount_NotKnown);
        DECLARE_ACCOUNT(2,   rewards_authority,             ReadOnly,   Signer,     KnownAccount_NotKnown);
        DECLARE_ACCOUNT(3,   vote_program_id,               ReadOnly,   NotSigner,  KnownAccount_VoteProgram);
    }
    DECLARE_ACCOUNTS_NUMBER(4);

    // Ensure that the instruction data is the correct size
    if (params->data_len != sizeof(SetCommissionInstructionData)) {
        return Error_InvalidDataSize;
    }

    // Cast instruction data
    const SetCommissionInstructionData *instruction_data = (const SetCommissionInstuctionData *) params->data;

    // This is the vote account manager state
    const VoteAccountManagerState *manager_account_state = (const VoteAccountManagerState *) manager_account->data;

    // Ensure that the provided rewards authority is the rewards authority of the manager account
    if (!SolPubkey_same(&(manager_account_state->rewards_authority), rewards_authority->key)) {
        return Error_InvalidAccount_First + 3;
    }

    // If commission caps are being enforced, then check to make sure that there are no violations
    if (manager_account_state->use_commission_caps) {
        // First, if there is a leave_epoch set, then it is not possible to change commission at all, ever
        if (manager_account_state->leave_epoch > 0) {
            return Error_LeaveEpochAlreadySet;
        }

        // Next, ensure that the commission is not greater than the max allowed commission
        if (instruction_data->commission > manager_account_state->max_commission) {
            return Error_CommissionTooLarge;
        }

        Clock clock;
        if (sol_get_clock_sysvar(&clock)) {
            return Error_FailedToGetClock;
        }

        // If the commission change epoch is less than the current epoch, then set commission_change_epoch and
        // commission_change_epoch_original_commission.
        if (manager_account_state->commission_change_epoch < clock->epoch) {
            manager_account_state->commission_change_epoch = clock->epoch;
            if (!get_vote_account_commission
                (vote_account, &(manager_account_state->commission_change_epoch_original_commission))) {
                return Error_InvalidAccount_First + 1;
            }
        }

        // If the commission change is too large, then the change is not allowed
        uint16_t max_allowed_commission =
            (uint16_t) manager_account_state->commission_change_epoch_original_commission +
            (uint16_t) instruction_data->max_commission_increase_per_epoch;

        if (max_allowed_commission > 100) {
            max_allowed_commission = 100;
        }

        if (instruction_data->commission > max_allowed_commission) {
            return Error_CommissionChangeTooLarge;
        }
    }

    // Issue a vote update commission instruction to set the commission of the vote account
    SolInstruction instruction;

    instruction.program_id = &(Constants.vote_program_pubkey);

    SolAccountMeta account_metas[] =
          ///   0. `[WRITE]` Vote account to be updated
        { { /* pubkey */ vote_account->key, /* is_writable */ true, /* is_signer */ false },
          ///   1. `[SIGNER]` Withdraw authority
          { /* pubkey */ manager_account->key, /* is_writable */ false, /* is_signer */ true } };

    instruction.accounts = account_metas;
    instruction.account_len = ARRAY_LEN(account_metas);

    VoteSetCommissionData data = { 5, instruction_data->commission };

    instruction.data = (uint8_t *) &data;
    instruction.data_len = sizeof(data);

    return sol_invoke_signed(&instruction, params->ka, params->ka_num, signer_seeds, 1);
}
