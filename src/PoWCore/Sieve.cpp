// Copyright (c) 2014-present The Gapcoin developers
// Copyright (c) 2017-present The Riecoin developers
// Released under the GNU General Public License v3, http://www.gnu.org/licenses/

#ifndef __STDC_FORMAT_MACROS 
#define __STDC_FORMAT_MACROS 
#endif
#ifndef __STDC_LIMIT_MACROS	
#define __STDC_LIMIT_MACROS	
#endif
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <math.h>
#include <gmp.h>

#include "Sieve.h"

// Returns whether the given index is a prime or not
#define is_prime(ary, i) !(ary[(i) >> 6ULL] & (1ULL << ((i) & 63ULL)))
// Sets x to the next greater number divisible by y
#define bound(x, y) ((((x) + (y) - 1) / (y)) * (y))

using namespace std;

/** Create a new Sieve */
Sieve::Sieve(PoWProcessor *pprocessor, uint64_t n_primes, uint64_t sievesize) {
	this->pprocessor       = pprocessor;
	this->sievesize        = bound(sievesize, sizeof(uint64_t)*8);
	this->found_primes     = 0;
	this->n_gaps           = 0;
	this->cur_n_gaps       = 0;
	this->tests            = 0;
	this->cur_tests        = 0;
	this->passed_time      = 1;
	this->cur_found_primes = 0;
	this->cur_passed_time  = 1;
	this->sieve            = (uint64_t*) malloc(this->sievesize/8);
	this->utils            = new PoWUtils();
	mpz_init(this->mpz_start);
	mpz_init(this->mpz_e);
	mpz_init(this->mpz_r);
	mpz_init_set_ui(this->mpz_two, 2);
	init_primes(n_primes);
	this->starts           = std::vector<uint64_t>(primes.size(), 0);
}

Sieve::~Sieve() {
	free(sieve);
	mpz_clear(mpz_start);
	mpz_clear(mpz_e);
	mpz_clear(mpz_r);
	mpz_clear(mpz_two);

	delete utils;
}

/** 
 * sieve for the given header hash 
 *
 * Sets the pow adder to a prime starting a gap greater than difficulty,
 * if found
 *
 * The Sieve works in two stages, first it checks every odd number
 * if it is divisible by one of the pre-calculated primes.
 * Then it uses the Fermat-test to test the remaining numbers.
 */
void Sieve::run_sieve(PoW *pow, vector<uint8_t> *offset) {
	// speed measurement
	uint64_t start_time = PoWUtils::gettime_usec();
	
	mpz_t mpz_offset;
	mpz_init_set_ui(mpz_offset, 0);

	if (offset != NULL)
		ary_to_mpz(mpz_offset, offset->data(), offset->size());

	// make sure offset (and later start) is divisible by two
	if (mpz_get_ui(mpz_offset) & 1ULL)
		mpz_add_ui(mpz_offset, mpz_offset, 1ULL);

	mpz_t mpz_adder, mpz_tmp;
	mpz_init(mpz_tmp);
	mpz_init(mpz_adder);

	pow->get_hash(mpz_start);
	mpz_mul_2exp(mpz_start, mpz_start, pow->get_shift());
	mpz_add(mpz_start, mpz_start, mpz_offset);

	// clear the sieve
	memset(sieve, 0, sievesize/8);

	// calculates for each prime, the first index in the sieve which is divisible by that prime */
	calc_muls();

	// Sieve all small primes (skip 2)
	for (uint64_t i(4); i < primes.size(); i++) {
		// Sieve all odd multiplies of the current prime
		for (uint64_t p(starts[i]); p < sievesize; p += primes2[i])
			sieve[p >> 6ULL] |= (1ULL << (p & 63ULL)); // Sets p as Composite.
	}

	// Make sure min_len is divisible by two
	uint64_t min_len = pow->target_size(mpz_start) & ~1ULL;
	uint64_t i       = 1ULL;
	uint64_t start   = sievesize + 4ULL;

	uint64_t offset3 = 3 - mpz_tdiv_ui(mpz_start, 3);
	uint64_t offset5 = 5 - mpz_tdiv_ui(mpz_start, 5);
	uint64_t offset7 = 7 - mpz_tdiv_ui(mpz_start, 7);

	// x mod n == 0: no offset, set to 0
	if (offset3 == 3) offset3 = 0;
	if (offset5 == 5) offset5 = 0;
	if (offset7 == 7) offset7 = 0;

	// Find the first prime
	for (; i < sievesize; i += 2) {
		if (is_prime(sieve, i)) {
			if ((i % 3) == offset3) continue;
			if ((i % 5) == offset5) continue;
			if ((i % 7) == offset7) continue;
			cur_tests++;
			tests++;
			mpz_add_ui(mpz_tmp, mpz_start, i);
			if (fermat_test(mpz_tmp))
				break;
		}
	}

	start = i;
	i    += min_len;

	// Scan the sieve in steps of size min_len
	bool finished = false;
	uint64_t n_test = 0;
	uint64_t gap_count = 0;
	while (i < sievesize && !finished) {
		// Scan the current gap
		for (; i > start; i -= 2) {
			if (is_prime(sieve, i)) {
				if ((i % 3) == offset3) continue;
				if ((i % 5) == offset5) continue;
				if ((i % 7) == offset7) continue;

				n_test++;
				mpz_add_ui(mpz_tmp, mpz_start, i);
		 
				if (fermat_test(mpz_tmp)) {
					start = i;
					i += min_len + 2;
					gap_count++;

					if (i >= sievesize) {
						i = 2;
						finished = true;
					}
				}
			}
		}

		if (!finished) {
			gap_count++;
			mpz_set_ui(mpz_adder, (uint64_t) start);
			mpz_add(mpz_adder, mpz_adder, mpz_offset);
 
			pow->set_adder(mpz_adder);
 
			if (pow->valid()) {
				if (pprocessor->process(pow))
					i = sievesize;
			}

			i += min_len << 1;
		}
	}

	mpz_clear(mpz_offset);
	mpz_clear(mpz_adder);
	mpz_clear(mpz_tmp);
	passed_time     += PoWUtils::gettime_usec() - start_time;
	cur_passed_time  = (cur_passed_time + 3 * (PoWUtils::gettime_usec() - start_time)) / 4;

	tests += n_test;
	cur_tests = (cur_tests + 3 * n_test) / 4;

	n_gaps += gap_count;
	cur_n_gaps = (cur_n_gaps + 3 * gap_count) / 4;

	// Approximate the number of primes within the sieve
	double log_start = log(mpz_get_d(mpz_start));
	cur_found_primes = (cur_found_primes + 3 * (sievesize / log_start)) / 4;
	found_primes += sievesize / log_start;

	if (debug && is_sieve_valid(i))
		printf("[DD] sieve check [PASSED]\n");
	else if (debug)
		printf("[EE] sieve check [FAILED]\n");
}

/** Returns the average primes per seconds */
double Sieve::avg_primes_per_sec() {
	if (passed_time < 10)
		return 0;
	return (((double)found_primes)*1000000.)/((double) passed_time);
}

/** Returns the primes per seconds */
double Sieve::primes_per_sec() {
	if (passed_time < 10)
		return 0;
	return (((double) cur_found_primes)*1000000.0L)/((double) cur_passed_time);
}

/** Returns the prime gaps per second */
double Sieve::gaps_per_second() {
	if (passed_time < 10)
		return 0;
	return (((double) cur_n_gaps)*1000000.0L)/((double) cur_passed_time);
}

/** Returns average the prime gaps per second */
double Sieve::avg_gaps_per_second() {
	if (passed_time < 10)
		return 0;
	return (((double) n_gaps)*1000000.0L)/((double) passed_time);
}

/** Returns the prime tests per second */
double Sieve::tests_per_second() {
	if (passed_time < 10)
		return 0;
	return (((double) cur_tests) * 1000000.0L)/((double) cur_passed_time);
}

/** Returns average the prime tests per second */
double Sieve::avg_tests_per_second() {
	if (passed_time < 10)
		return 0;
	return (((double) tests)*1000000.0L)/((double) passed_time);
}

/** Generates the first n primes using the sieve of Eratosthenes */
void Sieve::init_primes(uint64_t n) {
	const uint64_t limit(n*log(n) + n*log(log(n)));
	if (limit < 2) return;
	std::vector<uint64_t> compositeTable(limit/128ULL + 1ULL, 0ULL); // Booleans indicating whether an odd number is composite: 0000100100101100...
	for (uint64_t f(3ULL) ; f*f <= limit ; f += 2ULL) { // Eliminate f and its multiples m for odd f from 3 to square root of the limit
		if (compositeTable[f >> 7ULL] & (1ULL << ((f >> 1ULL) & 63ULL))) continue; // Skip if f is composite (f and its multiples were already eliminated)
		for (uint64_t m((f*f) >> 1ULL) ; m <= (limit >> 1ULL) ; m += f) // Start eliminating at f^2 (multiples of f below were already eliminated)
			compositeTable[m >> 6ULL] |= 1ULL << (m & 63ULL);
	}
	primes = {2};
	primes2 = {4};
	for (uint64_t i(1ULL) ; (i << 1ULL) + 1ULL <= limit && primes.size() < n ; i++) { // Fill the prime table using the composite table
		if (!(compositeTable[i >> 6ULL] & (1ULL << (i & 63ULL)))) {
			primes.push_back((i << 1ULL) + 1ULL); // Add prime number 2i + 1
			primes2.push_back(((i << 1ULL) + 1ULL) << 1ULL); // Add prime number 2i + 1 (x2)
		}
	}
}

/** Calculate for every prime the first index in the sieve which is divisible by that prime (and not divisible by two) */
void Sieve::calc_muls() {
	for (uint64_t i = 0; i < primes.size(); i++) {
		starts[i] = primes[i] - mpz_tdiv_ui(mpz_start, primes[i]);
		if (starts[i] == primes[i])
			starts[i] = 0;
		// is start index divisible by two (this check works because mpz_start is divisible by two)
		if ((starts[i] & 1) == 0)
			starts[i] += primes[i];
	}
}

/** Fermat pseudo prime test */
inline bool Sieve::fermat_test(mpz_t mpz_p) {
	// tmp = p - 1
	mpz_sub_ui(mpz_e, mpz_p, 1);
	// res = 2^tmp mod p
	mpz_powm(mpz_r, mpz_two, mpz_e, mpz_p);
	return mpz_cmp_ui(mpz_r, 1) == 0;
}

/** Verifies a given gap */
bool Sieve::is_gap_valid(uint64_t index, uint64_t length) {
	mpz_t mpz_start, mpz_end, mpz_len;
	mpz_init(mpz_len);
	mpz_init(mpz_end);

	mpz_init_set_ui(mpz_start, index);
	mpz_add(mpz_start, mpz_start, this->mpz_start);
	
	mpz_nextprime(mpz_end, mpz_start);

	mpz_sub(mpz_len, mpz_end, mpz_start);
	
	bool result = !mpz_cmp_ui(mpz_len, length);
	mpz_clear(mpz_start);
	mpz_clear(mpz_end);
	mpz_clear(mpz_len);

	return result;
}

/** Verifies that the sieve was sieved correctly */
bool Sieve::is_sieve_valid(uint64_t db_break) {
	mpz_t mpz_p;
	mpz_init(mpz_p);

	bool result = true;

	// run primality test for all remaining prime candidates
	for (uint64_t i = 1; i < db_break && i < sievesize && result; i += 2) {
		mpz_add_ui(mpz_p, mpz_start, i);
		// is_prime(sieve, i) <=> miller_rabin_test(start + i)
		result = !(is_prime(sieve, i) xor (mpz_probab_prime_p(mpz_p, 25) > 0));
	}

	mpz_clear(mpz_p);
	return result;
}
