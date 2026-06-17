// Copyright (c) 2014-present The Gapcoin developers
// Released under the GNU General Public License v3, http://www.gnu.org/licenses/

#ifndef __POWUTILS_H__
#define __POWUTILS_H__

#include <stdlib.h>
#include <inttypes.h>
#include <gmp.h>
#include <vector>
#include <string>

using namespace std;

// globally enable debugging
//#define DEBUG

/** Converts an byte array to an mpz value
 *
 * last significant word first
 * last significant byte first
 */
#define ary_to_mpz(mpz_res, ary, len) \
	mpz_import(mpz_res, len, -1, sizeof(uint8_t), -1, 0, ary)

/** Converts an mpz value to an byte array
 *
 * last significant word first
 * last significant byte first
 */
#define mpz_to_ary(mpz_src, ary, len) \
	mpz_export(ary, len, -1, sizeof(uint8_t), -1, 0, mpz_src)

// 2^48
#define TWO_POW48 (((uint64_t) 1) << 48)

/* converts an hex char to the lower byte nibble */
#define hex_to_nibble(hex) (((hex) > 57) ? (hex) - 87 : (hex) - 48)

/* converts an lower byte nibble to an hex char */
#define nibble_to_hex(nibble) (((nibble) >= 10) ? (nibble) + 87 : (nibble) + 48)

/* converts two chars of a hex string to a byte */
#define hex_to_byte(str, i) \
	((hex_to_nibble((str)->c_str()[i]) << 4) | hex_to_nibble((str)->c_str()[i + 1]))

/* sets the context of val from the given hex string */
#define set_from_hex(val, i, str, j) \
	((uint8_t *) &(val))[i] = hex_to_byte(str, j)
													

/* adds the context of ary at index i to an hex string */
#define ary_push_hex(str, val, i) do { \
	(str).push_back(nibble_to_hex((val)[i] >> 4)); \
	(str).push_back(nibble_to_hex((val)[i] & 0xf)); \
} while (0)

/* adds the context of val to an hex string */
#define push_hex(str, val, i) ary_push_hex(str, ((uint8_t *) &val), i)

/* converts an mpz to its hex representation */
inline string mpz_to_hex(mpz_t mpz) {
	string hex;
	size_t len;
	uint8_t *ary = (uint8_t *) mpz_to_ary(mpz, NULL, &len);

	/* swap order because of little endian format */
	for (size_t i = len; i > 0; i--)
		ary_push_hex(hex, ary, i - 1);
 
	return hex;
}

class PoWUtils {
public:
	// Should we debug
#ifdef DEBUG
	static const bool debug = true;
#else
	static const bool debug = false;
#endif

	/** The minimum difficulty */
	static const uint64_t min_difficulty = 16 * TWO_POW48;
	static const uint64_t min_test_difficulty = TWO_POW48;

	/** Returns the current time in microseconds */
	static uint64_t gettime_usec();

	/** Calculates the log2 from a mpz value. The return value is 2^accuracy times greater than the actual log2 value to provide a accuracy-bit accuracy */
	void mpz_log2(mpz_t mpz_log, mpz_t mpz_src, uint32_t accuracy);

	/** Calculates the merit of a given prime gap. The return value is 2^48 times greater than the actual merit value to provide a 48 bit accuracy */
	uint64_t merit(mpz_t mpz_start, mpz_t mpz_end);

	/** Generates a pseudo random number from the given gap */
	uint64_t rand(mpz_t mpz_start, mpz_t mpz_end);

	/** Generates the current difficulty, which is merit + random(start, end). The return value is 2^48 times greater than the actual merit + rand value to provide a 48 bit accuracy */
	uint64_t difficulty(mpz_t mpz_start, mpz_t mpz_end);

	/** Returns the given difficulty in human readable format */
	double get_readable_difficulty(uint64_t difficulty);

	/** Returns the target gap size for a given difficulty and start index */
	uint64_t target_size(mpz_t mpz_start, uint64_t difficulty);

	/** Calculates the next difficulty according to the given target and actual timespan
	*
	* calculates difficulty + log(target_timespan / actual_timespan)
	* 
	* Note: difficulty increases logarithmically: a 2,718(e) block speed increase is a +1 difficulty increase */
	uint64_t next_difficulty(uint64_t difficulty, 
														uint64_t actual_timespan,
														bool testnet);


	/** Compute the maximum difficulty decrease from the given difficulty in the given time */
	static uint64_t max_difficulty_decrease(uint64_t difficulty, int64_t time, bool testnet);

	/** Returns the estimated gaps (blocks) per day for the given primes per second and difficulty */
	double gaps_per_day(double pps, uint64_t difficulty);

	/** Allow more than one instance of this to be more cache friendly */
	PoWUtils();
	~PoWUtils();

private :
	/* log2(e) * 2^(64 + 48) */
	mpz_t mpz_log2e112;

	/* log2(e) * 2^64 */
	mpz_t mpz_log2e64;

	/* target spacing */
	static const int64_t target_spacing = 150;

	/* log(target_spacing) * 2^48 */
	static const uint64_t log_150_48 = 0x502b8fea053a6LL;

	/* 1.0/2^47 (for debugging) (not 48 bytes because rounding errors could count as failure) */
	constexpr static const double accuracy = 7.105427357601002e-15;

	/** Generates a pseudo random number from the given gap (double version for debugging) */
	double rand_d(mpz_t mpz_start, mpz_t mpz_end);

	/** Calculates the next difficulty according to the given target and actual timespan (double version for debugging) */
	double next_difficulty_d(double difficulty, uint64_t actual_timespan, bool testnet);

	/** Returns the estimated work required to find a gap with the given difficulty, which is e^difficulty (work are the total among of primes to calculate) */
	double target_work_d(uint64_t difficulty);
};

#endif /* __POWUTILS_H__ */
