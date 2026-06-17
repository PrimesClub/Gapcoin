// Copyright (c) 2014-present The Gapcoin developers
// Copyright (c) 2026-present The Riecoin developers
// Released under the GNU General Public License v3, http://www.gnu.org/licenses/

#ifndef __PRIME_H__
#define __PRIME_H__
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <gmp.h>

#include "PoW.h"
#include "PoWUtils.h"
#include "PoWProcessor.h"

using namespace std;

class Sieve {
	// primality testing
	mpz_t mpz_e, mpz_r, mpz_two;
public :
// should we debug
#ifdef DEBUG
	static const bool debug = true;
#else
	static const bool debug = false;
#endif

	/** Create a new Sieve */
	Sieve(PoWProcessor *pprocessor, uint64_t n_primes, uint64_t sievesize);

	~Sieve();

	/** Sets the PoWProcessor of this */
	void set_pprocessor(PoWProcessor *pprocessor) {this->pprocessor = pprocessor;}

	/** Sieve for the given header hash 
		*
		* returns an adder (within pow) starting a gap greater than difficulty
		* or NULL if no such prime was found
		*/
	void run_sieve(PoW *pow, vector<uint8_t> *offset);

	/** Returns the primes per seconds */
	double primes_per_sec();

	/** Returns the average primes per seconds */
	double avg_primes_per_sec();

	/** Returns the prime gaps per second */
	double gaps_per_second();

	/** Returns average the prime gaps per second */
	double avg_gaps_per_second();

	/** Returns the prime tests per second */
	double tests_per_second();

	/** Returns average the prime tests per second */
	double avg_tests_per_second();

	/** Returns the estimated gaps (blocks) per day */ 
	double gaps_per_day();

	/** Return the total number of found primes */
	uint64_t get_found_primes() {return found_primes;}

protected :
	/* First n primes */
	std::vector<uint64_t> primes;

	/* First n primes *2 */
	std::vector<uint64_t> primes2;

	/** Start indexes for each prime while sieving the current hash */
	std::vector<uint64_t> starts;

	/* sieve size in bits */
	uint64_t sievesize;

	/* the sieve as an ary of 64 bit words */
	uint64_t *sieve;

	/* the start of the sieve */
	mpz_t mpz_start;

	/* overall found primes */
	uint64_t found_primes;

	/* overall prime gaps */
	uint64_t n_gaps;

	/* current prime gaps */
	uint64_t cur_n_gaps;

	/* overall prime tests */
	uint64_t tests;

	/* current prime tests */
	uint64_t cur_tests;

	/* passed time mining */
	uint64_t passed_time;

	/* current found primes */
	uint64_t cur_found_primes;

	/* time passed since the last interval */
	uint64_t cur_passed_time;

	/* callback object to process an calculated PoW */
	PoWProcessor *pprocessor;

	/* PoW calculation utils */
	PoWUtils *utils;

	/** Generates the first n primes using the sieve of Eratosthenes */
	void init_primes(uint64_t n);

	/** calculate the sieve start indexes */
	void calc_muls();

	/** Fermat pseudo prime test */
	inline bool fermat_test(mpz_t mpz_p);

	/** Verifies a given gap */
	bool is_gap_valid(uint64_t index, uint64_t length);

	/** Verifies that the sieve was sieved correctly */
	bool is_sieve_valid(uint64_t db_break);
};

#endif /* __PRIME_H__ */
