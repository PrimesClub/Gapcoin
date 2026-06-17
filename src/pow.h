// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-present The Bitcoin Core developers
// Copyright (c) 2014-2021 The Gapcoin developers
// Copyright (c) 2013-present The Riecoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_POW_H
#define BITCOIN_POW_H

#include <consensus/params.h>
#include <gmp.h>
#include <gmpxx.h>

#include <cstdint>

class CBlockHeader;
class CBlockIndex;
class uint256;
class arith_uint256;

uint64_t GetNextWorkRequired(const CBlockIndex* pindexLast, const Consensus::Params& params);
uint64_t CalculateNextWorkRequired(const CBlockIndex* pindexLast, const Consensus::Params& params);

extern const std::vector<uint64_t> primeTable;
/** Check whether a Nonce satisfies the proof-of-work requirement */
bool CheckProofOfWork(uint256 hash, uint16_t nShift, const std::vector<uint8_t>& nAdd, uint64_t nDifficulty);
bool CheckProofOfWorkImpl(uint256 hash, uint16_t nShift, const std::vector<uint8_t>& nAdd, uint64_t nDifficulty);

#endif // BITCOIN_POW_H
