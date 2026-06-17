// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-present The Bitcoin Core developers
// Copyright (c) 2014-2021 The Gapcoin developers
// Copyright (c) 2013-present The Riecoin developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <kernel/chainparams.h>

#include <chainparamsseeds.h>
#include <consensus/amount.h>
#include <consensus/merkle.h>
#include <consensus/params.h>
#include <crypto/hex_base.h>
#include <hash.h>
#include <kernel/checkpointdata.h>
#include <kernel/messagestartchars.h>
#include <primitives/block.h>
#include <primitives/transaction.h>
#include <script/interpreter.h>
#include <script/script.h>
#include <script/verify_flags.h>
#include <uint256.h>
#include <util/chaintype.h>
#include <util/log.h>
#include <util/strencodings.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <map>
#include <span>
#include <utility>

using namespace util::hex_literals;

/** Build the genesis block. Note that the output of its generation transaction cannot be spent since it did not originally exist in the database. */
static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint64_t nTime, uint32_t nNonce, uint64_t nBits, int32_t nVersion, uint16_t nShift, std::vector<uint8_t> nAdd, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.version = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(MakeTransactionRef(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    genesis.nShift   = nShift;
    genesis.nAdd     = nAdd;
    return genesis;
}

void CChainParams::ApplyDeploymentOptions(const DeploymentOptions& opts)
{
    for (const auto& [deployment_pos, version_bits_params] : opts.version_bits_parameters) {
        consensus.vDeployments[deployment_pos].nStartTime = version_bits_params.start_time;
        consensus.vDeployments[deployment_pos].nTimeout = version_bits_params.timeout;
        consensus.vDeployments[deployment_pos].min_activation_height = version_bits_params.min_activation_height;
    }
}

/** Main network on which people trade goods and services. */
class CMainParams : public CChainParams {
public:
    CMainParams(const MainNetOptions& opts) {
        m_chain_type = ChainType::MAIN;
        consensus.nSubsidyHalvingInterval = 420000;
        consensus.MinBIP9WarningHeight = 2478151;
        consensus.nBitsMin = 16ULL << 48ULL;
        consensus.nPowTargetSpacing = 150; // 2.5 min
        consensus.fPowNoRetargeting = false;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 3024; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].period = 4032; // 7 days

        ApplyDeploymentOptions(opts.dep_opts);

        consensus.nMinimumChainWork = uint256{"00000000000000000000000000000000000000000000000000213622b9ddf093"}; // 2478511

        /** The message start string is designed to be unlikely to occur in normal data. The characters are rarely used upper ASCII, not valid as UTF-8, and produce a large 32-bit integer with any alignment. */
        pchMessageStart[0] = 0xd1;
        pchMessageStart[1] = 0xdf;
        pchMessageStart[2] = 0xe6;
        pchMessageStart[3] = 0xf9;
        nDefaultPort = 31469;
        nPruneAfterHeight = 100000;
        m_assumed_blockchain_size = 3;
        m_assumed_chain_state_size = 1;

        const CScript genesisOutputScript(CScript() << "044588d54931b7de2f9faaa5a3c1fde654114ae51273754e1f3f9720127f8977af6bfaa1f33e22e80e4b83f5269921501b411d254929faf1b10d2174ded28ac59d"_hex << OP_CHECKSIG);
        genesis = CreateGenesisBlock("The Times 15/Oct/2014 US data sends global stocks into tail-spin", genesisOutputScript, 1413914400, 13370, consensus.nBitsMin, 1, 20, {233, 156, 15}, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256{"e798f3ae4f57adcf25740fe43100d95ec4fd5d43a1568bc89e2b25df89ff6cb0"});
        assert(genesis.hashMerkleRoot == uint256{"261010cfad2ae26a355e56c2551ea2cc05549df11db7f40db7c2b9e3b40e1194"});

        // Note that of those which support the service bits prefix, most only support a subset of
        // possible options.
        // This is fine at runtime as we'll fall back to using them as an addrfetch if they don't support the
        // service bits we want, but we should get them updated to support all service bits wanted by any
        // release ASAP to avoid it where possible.
        // Todo: make/port Seeder for Riecoin and add Seeders here

        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};

        bech32_hrp = "gap";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_main), std::end(chainparams_seed_main));

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        checkpointData = mainCheckpointData;

        m_assumeutxo_data = {
            { // dumptxoutset Utxo.dat rollback '{"rollback": 2476000}'
                .height = mainCheckpointData.assumedValidBlockHeight,
                .hash_serialized = AssumeutxoHash{uint256{"79e06f27b58ba5449b7ecad54ffee3d000ed82b3ae070b17bda4cb0b7139adf7"}},
                .m_chain_tx_count = 2689488,
                .blockhash = mainCheckpointData.assumedValidBlockHash
            }
        };

        chainTxData = ChainTxData{
            // getchaintxstats 65536 ba868d8969736d149a34590c48446b2af5f425e2fc7665196fda7455a1dad1b2
            .nTime    = 1781172148,
            .tx_count = 2692000,
            .dTxRate  = 0.006719591242756351,
        };
    }
};

/** Testnet: public test network which is reset from time to time. */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams(const TestNetOptions& opts) {
        m_chain_type = ChainType::TESTNET;
        consensus.nSubsidyHalvingInterval = 420000;
        consensus.MinBIP9WarningHeight = 0;
        consensus.nBitsMin = 1ULL << 48ULL;
        consensus.nPowTargetSpacing = 300; // 5 min, 2x less blocks to download for TestNet
        consensus.fPowNoRetargeting = false;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = Consensus::BIP9Deployment::NEVER_ACTIVE;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 3024; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].period = 4032; // 7 days

        ApplyDeploymentOptions(opts.dep_opts);

        consensus.nMinimumChainWork = uint256{"0000000000000000000000000000000000000000000000000000000000000000"}; // 

        pchMessageStart[0] = 0x0b;
        pchMessageStart[1] = 0x11;
        pchMessageStart[2] = 0x09;
        pchMessageStart[3] = 0x07;
        nDefaultPort = 19661;
        nPruneAfterHeight = 1000;
        m_assumed_blockchain_size = 1;
        m_assumed_chain_state_size = 1;

        const CScript genesisOutputScript(CScript() << "044588d54931b7de2f9faaa5a3c1fde654114ae51273754e1f3f9720127f8977af6bfaa1f33e22e80e4b83f5269921501b411d254929faf1b10d2174ded28ac59d"_hex << OP_CHECKSIG);
        genesis = CreateGenesisBlock("The Times 15/Oct/2014 US data sends global stocks into tail-spin", genesisOutputScript, 1413914400, 1, consensus.nBitsMin, 1, 20, {25, 1}, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256{"cee5f695d016eda5137a820588ea1891eb107bb94daccff819849507e5bb17cc"});

        vFixedSeeds.clear();
        vSeeds.clear();
        // nodes with support for servicebits filtering should be at the top
        // Todo: make/port Seeder for Riecoin and add Seeders here

        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "tgap";

        vFixedSeeds = std::vector<uint8_t>(std::begin(chainparams_seed_test), std::end(chainparams_seed_test));

        fDefaultConsistencyChecks = false;
        m_is_mockable_chain = false;

        checkpointData = testCheckpointData;

        m_assumeutxo_data = {
        };

        chainTxData = ChainTxData{
            .nTime = 0,
            .tx_count = 0,
            .dTxRate = 0.001
        };
    }
};

/** Regression test: intended for private networks only. Has minimal difficulty to ensure that blocks can be found instantly. */
class CRegTestParams : public CChainParams
{
public:
    explicit CRegTestParams(const RegTestOptions& opts)
    {
        m_chain_type = ChainType::REGTEST;
        consensus.nSubsidyHalvingInterval = 150;
        consensus.MinBIP9WarningHeight = 0;
        consensus.nBitsMin = 16ULL << 48ULL;
        consensus.nPowTargetSpacing = 150; // 2.5 min
        consensus.fPowNoRetargeting = true; // No Difficulty Adjustment

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = Consensus::BIP9Deployment::NO_TIMEOUT;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].min_activation_height = 0; // No activation delay
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].threshold = 108; // 75%
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].period = 144; // Faster than normal for regtest (144 instead of 2016)

        consensus.nMinimumChainWork = uint256{};

        pchMessageStart[0] = 0xfa;
        pchMessageStart[1] = 0xbf;
        pchMessageStart[2] = 0xb5;
        pchMessageStart[3] = 0xda;
        nDefaultPort = 18444;
        nPruneAfterHeight = opts.fastprune ? 100 : 1000;
        m_assumed_blockchain_size = 0;
        m_assumed_chain_state_size = 0;

        ApplyDeploymentOptions(opts.dep_opts);

        const CScript genesisOutputScript(CScript() << "044588d54931b7de2f9faaa5a3c1fde654114ae51273754e1f3f9720127f8977af6bfaa1f33e22e80e4b83f5269921501b411d254929faf1b10d2174ded28ac59d"_hex << OP_CHECKSIG);
        genesis = CreateGenesisBlock("The Times 15/Oct/2014 US data sends global stocks into tail-spin", genesisOutputScript, 1413914400, 13370, consensus.nBitsMin, 1, 20, {233, 156, 15}, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256{"e798f3ae4f57adcf25740fe43100d95ec4fd5d43a1568bc89e2b25df89ff6cb0"});

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();
        vSeeds.emplace_back("dummySeed.invalid.");

        fDefaultConsistencyChecks = true;
        m_is_mockable_chain = true;

        checkpointData = {
            {},
            uint256{}, 0 // Assumed Valid Block
        };

        m_assumeutxo_data = {
            {   // For use by unit tests
                .height = 110,
                .hash_serialized = AssumeutxoHash{uint256{"86e9a1205b418b16dde3a18a78c730e30137e28466bda5dbf6b33ab8fc05447c"}},
                .m_chain_tx_count = 111,
                .blockhash = uint256{"6f75185cec002ac29d8f809d001e4a5b80f4a38176cb4b083d64f6fd20f0094a"}
            },
            {
                // For use by fuzz target src/test/fuzz/utxo_snapshot.cpp
                .height = 200,
                .hash_serialized = AssumeutxoHash{uint256{"17dcc016d188d16068907cdeb38b75691a118d43053b8cd6a25969419381d13a"}},
                .m_chain_tx_count = 201,
                .blockhash = uint256{"385901ccbd69dff6bbd00065d01fb8a9e464dede7cfe0372443884f9b1dcf6b9"}
            },
            {
                // For use by test/functional/feature_assumeutxo.py and test/functional/tool_bitcoin_chainstate.py
                .height = 299,
                .hash_serialized = AssumeutxoHash{uint256{"0c4b0d858bcdfbcb68adfb3563908572899112aa3c6b5417577ca55a6f28436c"}},
                .m_chain_tx_count = 334,
                .blockhash = uint256{"606d092fdc8b336dde68fb3d3d850d1e80c58de1cc6abda4549290d79490d2a0"}
            },
        };

        chainTxData = ChainTxData{
            .nTime = 0,
            .tx_count = 0,
            .dTxRate = 0.001, // Set a non-zero rate to make it testable
        };

        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x35, 0x87, 0xCF};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x35, 0x83, 0x94};

        bech32_hrp = "rric"; // Use same as Riecoin to avoid having to adjust Tests again.
    }
};

std::unique_ptr<const CChainParams> CChainParams::RegTest(const RegTestOptions& options)
{
    return std::make_unique<const CRegTestParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::Main(const MainNetOptions& options)
{
    return std::make_unique<const CMainParams>(options);
}

std::unique_ptr<const CChainParams> CChainParams::TestNet(const TestNetOptions& options)
{
    return std::make_unique<const CTestNetParams>(options);
}

std::vector<int> CChainParams::GetAvailableSnapshotHeights() const
{
    std::vector<int> heights;
    heights.reserve(m_assumeutxo_data.size());

    for (const auto& data : m_assumeutxo_data) {
        heights.emplace_back(data.height);
    }
    return heights;
}

std::optional<ChainType> GetNetworkForMagic(const MessageStartChars& message)
{
    const auto mainnet_msg = CChainParams::Main()->MessageStart();
    const auto testnet_msg = CChainParams::TestNet()->MessageStart();
    const auto regtest_msg = CChainParams::RegTest()->MessageStart();

    if (std::ranges::equal(message, mainnet_msg)) {
        return ChainType::MAIN;
    } else if (std::ranges::equal(message, testnet_msg)) {
        return ChainType::TESTNET;
    } else if (std::ranges::equal(message, regtest_msg)) {
        return ChainType::REGTEST;
    }
    return std::nullopt;
}
