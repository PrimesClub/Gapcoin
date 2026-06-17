// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-present The Bitcoin Core developers
// Copyright (c) 2014-2021 The Gapcoin developers
// Copyright (c) 2013-present The Riecoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <pow.h>

#include <arith_uint256.h>
#include <chain.h>
#include <logging.h>
#include <primitives/block.h>
#include <uint256.h>
#include <util/check.h>

static PoWUtils *powUtils = new PoWUtils();

// Minimum amount of work that could possibly be required nTime after minimum work required was nBase
uint64_t ComputeMinWork(uint64_t nBase, int64_t nTime)
{
    return PoWUtils::max_difficulty_decrease(nBase, nTime, false);
}

uint64_t GetNextWorkRequired(const CBlockIndex* pindexLast, const Consensus::Params& params) {
	return CalculateNextWorkRequired(pindexLast, params);
}
uint64_t CalculateNextWorkRequired(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    // Genesis block
    if (pindexLast == NULL)
        return (PoWUtils::min_difficulty);

    // don't use genesis block
    const CBlockIndex* pindexFirst = pindexLast;
    if (pindexFirst->pprev && pindexFirst->pprev->pprev)
        pindexFirst = pindexFirst->pprev;

    // Limit adjustment step
    int64_t nActualTimespan(pindexLast->GetBlockTime() - pindexFirst->GetBlockTime());

    // if prev block not avilable asume optimal timespan
    if (pindexFirst == pindexLast)
        nActualTimespan = params.nPowTargetSpacing;

    // do not divide by zero (or negative number)
    if (nActualTimespan < 1)
      nActualTimespan = 1;

    // Retarget
    uint64_t nextDifficulty = powUtils->next_difficulty(pindexLast->nBits, nActualTimespan, false);

    /// debug print
    /*LogPrintf("GetNextWorkRequired RETARGET\n");
    LogPrintf("nTargetTimespan = %d    nActualTimespan = %d\n", nTargetTimespan, nActualTimespan);
    LogPrintf("Before: %016llx  %F\n", pindexLast->nDifficulty, powUtils->get_readable_difficulty(pindexLast->nDifficulty));;
    LogPrintf("After:  %016llx  %F\n", nextDifficulty, powUtils->get_readable_difficulty(nextDifficulty));*/

    return nextDifficulty;
}

// Bypasses the actual proof of work check during fuzz testing .
bool CheckProofOfWork(uint256 hash, uint16_t nShift, const std::vector<uint8_t> &nAdd, const uint64_t nBits)
{
    if (EnableFuzzDeterminism()) return true;
    return CheckProofOfWorkImpl(hash, nShift, nAdd, nBits);
}

bool CheckProofOfWorkImpl(uint256 hash, uint16_t nShift, const std::vector<uint8_t> &nAdd, const uint64_t nBits)
{
    std::vector<uint8_t> vHash(hash.begin(), hash.end());

    PoW pow(vHash, nShift, nAdd, nBits);

    // Check proof of work matches claimed amount
    if (!pow.valid()) {
        LogError("CheckProofOfWork() : hash does not match nDifficulty");
        return false;
    }

    return true;
}
