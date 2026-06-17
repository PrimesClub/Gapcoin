// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-present The Bitcoin Core developers
// Copyright (c) 2013-present The Riecoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <primitives/block.h>

#include <hash.h>
#include <tinyformat.h>

#include <memory>
#include <sstream>

uint256 CBlockHeader::GetHash() const
{
    return (HashWriter{} << nVersion << hashPrevBlock << hashMerkleRoot << nTime << nBits << nNonce).GetHash();
}

std::string CBlock::ToString() const
{
    std::stringstream s;
    s << strprintf("CBlock(hash=%s, ver=0x%08x, hashPrevBlock=%s, hashMerkleRoot=%s, nTime=%u, nBits=0x%08x, nNonce=%u, nShift=%u, nAdd",
        GetHash().ToString(),
        nVersion,
        hashPrevBlock.ToString(),
        hashMerkleRoot.ToString(),
        nTime, nBits, nNonce, nShift);
    for (const auto& na : nAdd)
        s << " " << static_cast<uint16_t>(na);
    s << strprintf(", vtx=%u)\n", vtx.size());
    for (const auto& tx : vtx) {
        s << "  " << tx->ToString() << "\n";
    }
    return s.str();
}
