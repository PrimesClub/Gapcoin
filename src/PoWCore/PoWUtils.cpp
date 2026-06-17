// Copyright (c) 2014-present The Gapcoin developers
// Released under the GNU General Public License v3, http://www.gnu.org/licenses/

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <array>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <sys/time.h>
#include "picosha2.h"
#include <gmp.h>
#include "PoWUtils.h"

using namespace std;

/** Calculates the log2 from a mpz value, the return value is 2^accuracy times greater than the actual log2 value to provide a finer accuracy */
void PoWUtils::mpz_log2(mpz_t mpz_log, mpz_t mpz_src, uint32_t accuracy) {
	mpz_t mpz_tmp, mpz_n;
	mpz_init(mpz_tmp);
	mpz_init_set(mpz_n, mpz_src);
	
	// log2 without the decimal part
	mpz_set_ui(mpz_log, mpz_sizeinbase(mpz_n, 2) - 1);

	uint32_t bits = 0;
	uint32_t shift = accuracy + mpz_get_ui(mpz_log);

	/* add accuracy bits */
	mpz_mul_2exp(mpz_log, mpz_log, accuracy);
	mpz_mul_2exp(mpz_n,	 mpz_n,	 accuracy);

	for (;;) {

		mpz_div_2exp(mpz_tmp, mpz_n, shift);
		
		// while n / 2^accuracy < 2
		while (mpz_get_ui(mpz_tmp) < 2 && bits <= accuracy) {
			// n <- n^2
			mpz_mul(mpz_n, mpz_n, mpz_n);
			// Preserve accuracy
			mpz_div_2exp(mpz_n, mpz_n, shift);
			mpz_div_2exp(mpz_tmp, mpz_n, shift);
			bits++;
		}

		if (bits > accuracy) break;

		// log += 2^(accuracy - bits)
		mpz_set_ui(mpz_tmp, 1);
		mpz_mul_2exp(mpz_tmp, mpz_tmp, accuracy - bits);
		mpz_add(mpz_log, mpz_log, mpz_tmp);

		// n <- n/2
		mpz_div_2exp(mpz_n, mpz_n, 1);
	}

	mpz_clear(mpz_tmp);
	mpz_clear(mpz_n);
}

/** Calculates the merit of a given prime gap
 *
 * merit = gapsize/log(gapstart) = (gapsize * log2(e)) / log2(gapstart)
 *
 * the return value is 2^48 times greater than the actual merit value to provide a 48 bit accuracy
 */
uint64_t PoWUtils::merit(mpz_t mpz_start, mpz_t mpz_end) {
	mpz_t mpz_merit, mpz_ld;
	mpz_init(mpz_merit);
	mpz_init(mpz_ld);

	// merit = gaplen * log2(e) * 2^(64 + 48)
	mpz_sub(mpz_merit, mpz_end, mpz_start);
	mpz_mul(mpz_merit, mpz_merit, mpz_log2e112);

	// merit = merit / (log2(gapstart) * 2^64)
	mpz_log2(mpz_ld, mpz_start, 64);
	mpz_div(mpz_merit, mpz_merit, mpz_ld);

	size_t datalen(0);
	uint8_t *data(reinterpret_cast<uint8_t*>(mpz_export(nullptr, &datalen, -1, sizeof(uint8_t), 0, 0, mpz_merit)));
	uint64_t merit(0);
	for (size_t i(0); i < datalen && i < 8; i++)
		reinterpret_cast<uint8_t*>(&merit)[i] = data[i];

	mpz_clear(mpz_merit);
	mpz_clear(mpz_ld);
	free(data);

	return merit;
}

inline std::array<uint8_t, 32> sha256(const uint8_t *data, uint32_t len) {
	std::array<uint8_t, 32> hash;
	picosha2::hash256(data, data + len, hash.begin(), hash.end());
	return hash;
}
inline std::array<uint8_t, 32> sha256d(const uint8_t *data, uint32_t len) {
	return sha256(sha256(data, len).data(), 32);
}

/** Generates a pseudo random number from the given gap */
uint64_t PoWUtils::rand(mpz_t mpz_start, mpz_t mpz_end) {
	uint8_t *start, *end;
	size_t start_len = 0, end_len = 0;
	std::vector<uint8_t> data;
	
	start = (uint8_t *) mpz_to_ary(mpz_start, NULL, &start_len);
	end   = (uint8_t *) mpz_to_ary(mpz_end, NULL, &end_len);
	for (size_t i(0); i < start_len; i++)
		data.push_back(start[i]);
	for (size_t i(0); i < end_len; i++)
		data.push_back(end[i]);
	free(start);
	free(end);
	
	std::array<uint8_t, 32> hash(sha256d(data.data(), data.size()));
	
	/* generate an uint64_t value form the 256 bit hash */
	uint64_t rand(reinterpret_cast<uint64_t*>(hash.data())[0]);

	// xor the 64 bit parts of the hash	
	for (size_t i(1); i < 4; i++)
		rand ^= reinterpret_cast<uint64_t*>(hash.data())[i];
	
	return rand;
}

/** Generates a pseudo random number from the given gap (double version for debugging) */
double PoWUtils::rand_d(mpz_t mpz_start, mpz_t mpz_end) {
	/* uint8_t tmp[SHA256_DIGEST_LENGTH];																	 
	uint8_t hash[SHA256_DIGEST_LENGTH];																	 
	
	uint8_t *start, *end;
	size_t start_len = 0, end_len = 0;

	start = (uint8_t *) mpz_to_ary(mpz_start, NULL, &start_len);
	end	 = (uint8_t *) mpz_to_ary(mpz_end,	NULL, &end_len);

	SHA256_CTX sha256;																													
	SHA256_Init(&sha256);																											 
	SHA256_Update(&sha256, start, start_len);																	 
	SHA256_Update(&sha256, end, end_len);																	 
	SHA256_Final(tmp, &sha256); 

	// hash the result again
	SHA256_Init(&sha256);																											 
	SHA256_Update(&sha256, tmp, SHA256_DIGEST_LENGTH);	
	SHA256_Final(hash, &sha256);

	uint32_t rand, i, *ptr = (uint32_t *) hash;

	for (i = 1, rand = ptr[0]; 
			 i < SHA256_DIGEST_LENGTH / sizeof(uint32_t); 
			 i++) {
		
		rand ^= ptr[i];
	}

	free(start);
	free(end);
	
	return ((double) rand) / ((double) UINT32_MAX);*/
	return 0.;
}

/** Generates the difficulty of a given prime gap */
uint64_t PoWUtils::difficulty(mpz_t mpz_start, mpz_t mpz_end) {
	mpz_t mpz_ld, mpz_tmp;
	mpz_init(mpz_ld);

	/* tmp = 2 * log2(e) * 2^(64 + 48) */
	mpz_init_set_ui(mpz_tmp, 2);
	mpz_mul(mpz_tmp, mpz_tmp, mpz_log2e112);

	/* tmp corresponds to 2 / log(start) with 64 bit accuracy*/
	mpz_log2(mpz_ld, mpz_start, 64);
	mpz_div(mpz_tmp, mpz_tmp, mpz_ld);

	// We just calculated the merit of the (average) minimal distance till the next greater merit for the given prime gap */
	size_t datalen(0);
	uint8_t *data(reinterpret_cast<uint8_t*>(mpz_export(nullptr, &datalen, -1, sizeof(uint8_t), 0, 0, mpz_tmp)));
	uint64_t min_gap_distance_merit(1);
	for (size_t i(0); i < datalen && i < 8; i++)
		reinterpret_cast<uint8_t*>(&min_gap_distance_merit)[i] = data[i];

	mpz_clear(mpz_ld);
	mpz_clear(mpz_tmp);
	free(data);

	// To refine the decimal part between the next greater merit we use an CSPRNG (cryptographically secure pseudo random number generator)
	uint64_t difficulty = merit(mpz_start, mpz_end) + (rand(mpz_start, mpz_end) % min_gap_distance_merit);

	// difficulty = gap_size / log(start) + rand(start, end) % merit_of_distance_to_next_gap
	return difficulty;
}

/** Returns the given difficulty in human readable format */
double PoWUtils::get_readable_difficulty(uint64_t difficulty) {
	return ((double) difficulty) / TWO_POW48;
}

/** Create a new PoWUtils object */
PoWUtils::PoWUtils() {
	// log2(e)*2^(64 + 48)
	mpz_init_set_str(mpz_log2e112, "171547652b82fe1777d0ffda0d23a", 16);
	// log2(e)*2^64
	mpz_init_set_str(mpz_log2e64, "171547652b82fe177", 16);
}

PoWUtils::~PoWUtils() {
	mpz_clear(mpz_log2e112);
	mpz_clear(mpz_log2e64);
}

/** Returns the target gap size for a given difficulty and start index
 *
 * difficulty * log(start) 
 * = (difficulty * 2^48 * log2(start) * 2^64) / (log2(e) * 2^(48 + 64))
 */
uint64_t PoWUtils::target_size(mpz_t mpz_start, uint64_t difficulty) {
	mpz_t mpz_target_size, mpz_difficulty;
	mpz_init(mpz_target_size);
	mpz_init(mpz_difficulty);
	mpz_import(mpz_difficulty, 1,  -1, sizeof(uint64_t), 0, 0, &difficulty);

	// target_size = (difficulty * log2(start)) / log2(e)
	mpz_log2(mpz_target_size, mpz_start, 64);
	mpz_mul(mpz_target_size, mpz_target_size, mpz_difficulty);
	mpz_div(mpz_target_size, mpz_target_size, mpz_log2e112);

	size_t datalen(0);
	uint8_t *data(reinterpret_cast<uint8_t*>(mpz_export(nullptr, &datalen, -1, sizeof(uint8_t), 0, 0, mpz_target_size)));
	uint64_t target_size(0);
	for (size_t i(0); i < datalen && i < 8; i++)
		reinterpret_cast<uint8_t*>(&target_size)[i] = data[i];

	mpz_clear(mpz_target_size);
	mpz_clear(mpz_difficulty);
	free(data);

	return target_size;
}

/** Returns the estimated work required to find a gap with the given difficulty, which is e^difficulty (work are the total among of primes to calculate). */
double PoWUtils::target_work_d(uint64_t difficulty) {
	double ddifficulty = ((double) difficulty) / TWO_POW48;
	double work = exp(ddifficulty);
	return work;
}

/** Returns the current time in microseconds */
uint64_t PoWUtils::gettime_usec() {
	struct timeval time;
	if (gettimeofday(&time, NULL) == -1)
		return ((uint64_t) -1);
	return ((uint64_t) time.tv_sec) * ((uint64_t) 1000000) + ((uint64_t) time.tv_usec);
}

/** Calculates the next difficulty according to 
 * the given target and actual timespan between the last two blocks
 * the difficulty is only changed about 1/256 of the 
 * actual increase and about 1/64 of the actual decrease
 *
 * calculates difficulty + log(target_timespan / actual_timespan)
 * <=> d + log(t / a) 
 *	 = d + log(t) - log(a)
 *	 = d + log2(t) / log2(e) - log2(a) / log2(e)
 * 
 * difficulty increases logarithmically:
 *	 a 2,718(e) block speed increase is a +1 difficulty increase */
uint64_t PoWUtils::next_difficulty(uint64_t difficulty, uint64_t actual_timespan, bool testnet) {
	// Calculate log(actual_timespan) * 2^48
	mpz_t mpz_log_actual;
	mpz_init_set_ui(mpz_log_actual, actual_timespan);
	
	// log_actual = (log2(actual_timespan) * 2^(64 + 48)) / (log2(e) * 2^64)
	mpz_log2(mpz_log_actual, mpz_log_actual, 64 + 48);
	mpz_div(mpz_log_actual, mpz_log_actual, mpz_log2e64);

	const uint64_t log_target = log_150_48;
	size_t datalen(0);
	uint8_t *data(reinterpret_cast<uint8_t*>(mpz_export(nullptr, &datalen, -1, sizeof(uint8_t), 0, 0, mpz_log_actual)));
	uint64_t log_actual(0);
	for (size_t i(0); i < datalen && i < 8; i++)
		reinterpret_cast<uint8_t*>(&log_actual)[i] = data[i];

	mpz_clear(mpz_log_actual);
	free(data);

	uint64_t next = difficulty;
	uint64_t shift = 8;

	// Correct hash rate lose faster (with out this the difficulty would mostly adjust to high)
	if (log_actual > log_target)
		shift = 6;

	next += log_target >> shift;
	next -= log_actual >> shift;

	// Avoid difficulty underflow
	if (log_actual > log_target && difficulty < ((log_actual >> shift) - (log_target >> shift)))
		next = (testnet ? min_test_difficulty : min_difficulty);

	// This should never happen, but avoid difficulty overflow
	if (log_actual < log_target && UINT64_MAX - ((log_target >> shift) - (log_actual >> shift)) < difficulty)
		next = UINT64_MAX;


	// Difficulty can only change about +/- 1 per block
	if (next > difficulty + TWO_POW48)
		next = difficulty + TWO_POW48;
	if (next < difficulty - TWO_POW48)
		next = difficulty - TWO_POW48;

	if (testnet && next < min_test_difficulty)
		next = min_test_difficulty;
	else if (!testnet && next < min_difficulty)
		next = min_difficulty;

	if (debug && abs(((double) next) / TWO_POW48 - next_difficulty_d(((double) difficulty) / TWO_POW48, actual_timespan, testnet)) > accuracy)
		printf("[EE] next_difficulty check [FAILED]\n");
	else if (debug)
		printf("[DD] next_difficulty check [PASSED]\n");
	return next;
}


/** Calculates the next difficulty according to  the given target and actual timespan
 *
 * calculates difficulty + log(target_timespan / actual_timespan)
 * 
 * difficulty increases logarithmically: a 2,718(e) block speed increase is a +1 difficulty increase */
double PoWUtils::next_difficulty_d(double difficulty, uint64_t actual_timespan, bool testnet) {
	uint64_t shift = 8;

	// Correct hash rate lose faster
	if (actual_timespan > 150)
		shift = 6;
		
	double next = difficulty + log(150.0 / ((double) actual_timespan)) / (1 << shift);

	// Difficulty can only change about +/- 1 per block
	if (next > difficulty + 1.0)
		next = difficulty + 1.0;
	if (next < difficulty - 1.0)
		next = difficulty - 1.0;

	if (testnet && next < ((double) min_test_difficulty) / TWO_POW48)
		next = ((double) min_test_difficulty) / TWO_POW48;
	else if (!testnet && next < ((double) min_difficulty) / TWO_POW48)
		next = ((double) min_difficulty) / TWO_POW48;

	return next;
}

/** Compute the maximum possible difficulty decrease from the given difficulty in the given time */
uint64_t PoWUtils::max_difficulty_decrease(uint64_t difficulty, int64_t time, bool testnet) {
	while (time > 0 && difficulty > min_difficulty) {
		// Difficulty can max decrease about +/- 1 which is factor ~174
		if (difficulty >= TWO_POW48)
			difficulty -= TWO_POW48;
		time -= 26100; // 174 * 150
	}

	if (difficulty < min_difficulty)
		difficulty = min_difficulty;

	return difficulty;
}

/** Returns the estimated gaps (blocks) per day for the given primes per second and difficulty */
double PoWUtils::gaps_per_day(double pps, uint64_t difficulty) {
	return (60*60*24)/(target_work_d(difficulty)/pps);
}
