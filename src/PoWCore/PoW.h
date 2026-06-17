// Copyright (c) 2014-present The Gapcoin developers
// Released under the GNU General Public License v3, http://www.gnu.org/licenses/

#ifndef __POW_H__
#define __POW_H__

#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <limits.h>
#include <gmp.h>
#include <gmpxx.h>
#include <vector>
#include <string>
#include "PoWUtils.h"

/** Compile time opt-out protection from dos attacks with high shift */
#define MAX_SHIFT 1024

using namespace std;

class PoW {
public :
	/** Create a new PoW out of the given hash, shift, adder and difficulty in calculation format */
	PoW(mpz_t mpz_hash, uint16_t shift, mpz_t mpz_adder, uint64_t difficulty, uint32_t nonce = 0);
	/** Create a new PoW out of the given hash, shift, adder and difficulty in block header format */
	PoW(const vector<uint8_t> &hash, const uint16_t shift, const vector<uint8_t> &adder, const uint64_t difficulty, uint32_t nonce = 0);
	~PoW();
	
	/** returns the uint64 difficulty */
	uint64_t difficulty();
	/** returns the uint64 merit */
	uint64_t merit();
	/** returns the vector gap */
	bool get_gap(mpz_class &start, mpz_class &end);
	
	/** returns the gap length for this PoW */
	uint64_t gap_len();
	
	/** returns whether this PoW is valid or not */
	bool valid() {return difficulty() >= target_difficulty;}
	
	/** returns the target min gap size for a given start */
	uint64_t target_size(mpz_t mpz_start) {return utils->target_size(mpz_start, target_difficulty);}
	
	/* returns a string representation of this */
	string to_s();
	
	/*****************************/
	/* getter and setter methods */
	/*****************************/
	void     get_hash(mpz_t mpz_hash) {mpz_set(mpz_hash, this->mpz_hash);}
	uint16_t get_shift() {return shift;}
	uint32_t get_nonce() {return nonce;}
	void     set_shift(const uint16_t shift) {this->shift = shift;}
	void     get_adder(mpz_t mpz_adder) {mpz_set(mpz_adder, this->mpz_adder);}
	void     get_adder(vector<uint8_t> *adder);
	void     set_adder(mpz_t mpz_adder) {mpz_set(this->mpz_adder, mpz_adder);}
	void     set_adder(vector<uint8_t> *adder);
	uint64_t get_target() {return target_difficulty;}
	
private :
	// the block header hash
	mpz_t mpz_hash;
	// block nonce (for compatibility)
	uint32_t nonce;
	/* the shift amount */
	uint16_t shift;
	/* the adder to the hash */
	mpz_t mpz_adder;
	/* the target difficulty */
	uint64_t target_difficulty;
	/* PoW calculation utils */
	PoWUtils *utils;
	
	/** Calculates the start and end prime for this pow. Returns whether the start and end were calculated correctly */
	bool get_end_points(mpz_t mpz_start, mpz_t mpz_end);
};

#endif
