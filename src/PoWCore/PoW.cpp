// Copyright (c) 2014-present The Gapcoin developers
// Released under the GNU General Public License v3, http://www.gnu.org/licenses/

#ifndef __STDC_FORMAT_MACROS 
#define __STDC_FORMAT_MACROS 
#endif
#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdlib.h>
#include <inttypes.h>
#include <gmp.h>
#include <sstream>
#include <string>
#include "PoW.h"

using namespace std;

/** Create a new PoW out of the given hash, shift, adder and difficulty in calculation format */
PoW::PoW(mpz_t mpz_hash, uint16_t shift, mpz_t mpz_adder, uint64_t difficulty, uint32_t nonce) {
	if (mpz_hash == NULL)
		mpz_init_set_ui(this->mpz_hash, 0);
	else
		mpz_init_set(this->mpz_hash, mpz_hash);

	if (mpz_adder == NULL)
		mpz_init_set_ui(this->mpz_adder, 0);
	else
		mpz_init_set(this->mpz_adder, mpz_adder);

	this->nonce = nonce;
	this->shift = shift;
	this->utils = new PoWUtils();

	target_difficulty = difficulty;
}

/** Create a new PoW out of the given hash, shift, adder and difficulty in block header format */
PoW::PoW(const vector<uint8_t> &hash, const uint16_t shift, const vector<uint8_t> &adder, const uint64_t difficulty, uint32_t nonce) {
	mpz_init_set_ui(mpz_hash, 0);
	mpz_init_set_ui(mpz_adder, 0);
	this->nonce = nonce;
	this->shift = shift;
	this->utils = new PoWUtils();

	ary_to_mpz(mpz_hash, hash.data(), hash.size());
	ary_to_mpz(mpz_adder, adder.data(), adder.size());

	target_difficulty = difficulty;
}

PoW::~PoW() {
	mpz_clear(mpz_hash);
	mpz_clear(mpz_adder);
	delete utils;
}

/** Calculates the start and end prime for this pow, returns whether the start and end calculated correctly */
bool PoW::get_end_points(mpz_t mpz_start, mpz_t mpz_end) {
	// shift hast to be greater or equal than 14
	if (shift < 14)
		return false;

// Compile time opt-in protection from dos attacks with high shift
#ifdef MAX_SHIFT
	if (shift > MAX_SHIFT)
		return false;
#endif

	/** make sure that hash is in range (2^255, 2^256) */
	if (mpz_sizeinbase(mpz_hash, 2) != 256)
		return false;

	/** make sure adder is smaller than 2^shift */
	if (mpz_sizeinbase(mpz_adder, 2) > shift)
		return false;

	mpz_init_set(mpz_start, mpz_hash);
	mpz_mul_2exp(mpz_start, mpz_start, shift);
	mpz_add(mpz_start, mpz_start, mpz_adder);

	/* start has to be a prime */
	if (!mpz_probab_prime_p(mpz_start, 25)) {
		mpz_clear(mpz_start);
		return false;
	}

	mpz_init(mpz_end);
	mpz_nextprime(mpz_end, mpz_start);

	return true;
}

/** returns the uint64 difficulty */
uint64_t PoW::difficulty() {
	mpz_t mpz_start, mpz_end;
	if (!get_end_points(mpz_start, mpz_end))
		return 0;

	uint64_t diff = utils->difficulty(mpz_start, mpz_end);

	mpz_clear(mpz_start);
	mpz_clear(mpz_end);

	return diff;
}

/** returns the uint64 merit */
uint64_t PoW::merit() {
	mpz_t mpz_start, mpz_end;
	if (!get_end_points(mpz_start, mpz_end))
		return 0;

	uint64_t merit = utils->merit(mpz_start, mpz_end);

	mpz_clear(mpz_start);
	mpz_clear(mpz_end);

	return merit;
}

/** returns the vector gap */
bool PoW::get_gap(mpz_class &start, mpz_class &end) {
	if (!get_end_points(start.get_mpz_t(), end.get_mpz_t())) {
		start = 0;
		end = 0;
		return false;
	}
	return true;
}

/** returns the gap length for this PoW */
uint64_t PoW::gap_len() {
	mpz_t mpz_start, mpz_end;
	if (!get_end_points(mpz_start, mpz_end))
		return 0;
	
	mpz_t mpz_len;
	mpz_init(mpz_len);
	
	mpz_sub(mpz_len, mpz_end, mpz_start);
	
	size_t datalen(0);
	uint8_t *data(reinterpret_cast<uint8_t*>(mpz_export(nullptr, &datalen, -1, sizeof(uint8_t), 0, 0, mpz_len)));
	uint64_t len(0);
	for (size_t i(0); i < datalen && i < 8; i++)
		reinterpret_cast<uint8_t*>(&len)[i] = data[i];
	
	mpz_clear(mpz_len);
	mpz_clear(mpz_start);
	mpz_clear(mpz_end);
	free(data);
	return len;
}

void PoW::get_adder(vector<uint8_t> *adder) {
	size_t len;
	uint8_t *ary = (uint8_t *) mpz_to_ary(mpz_adder, NULL, &len);
	adder->assign(ary, ary + len);
	free(ary);
}

void PoW::set_adder(vector<uint8_t> *adder) {
	if (adder != NULL)
		ary_to_mpz(mpz_adder, adder->data(), adder->size());
}

/* returns a string representation of this */
string PoW::to_s() {
	stringstream ss;

	ss << "PoW: " << (valid() ? "valid" : "invalid") << "\n";
	ss << "	hash:	" << mpz_to_hex(mpz_hash) << "\n";
	ss << "	nonce: " << nonce << "\n";
	ss << "	shift: " << shift << "\n";
	ss << "	adder: " << mpz_to_hex(mpz_adder) << "\n";
	ss << "	diff:	" << target_difficulty << "\n";

	mpz_t mpz_start, mpz_end;
	if (get_end_points(mpz_start, mpz_end)) {
		ss << "---------\n";
		ss << "	start: " << mpz_to_hex(mpz_start) << "\n";
		ss << "	end:	 " << mpz_to_hex(mpz_end) << "\n";
		ss << "	len:	 " << gap_len() << "\n";
		ss << "	merit: " << (((double) merit()) / TWO_POW48) << "\n";
		mpz_clear(mpz_start);
		mpz_clear(mpz_end);
	}

	return ss.str();
}
