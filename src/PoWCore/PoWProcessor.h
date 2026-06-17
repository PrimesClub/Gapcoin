// Copyright (c) 2014-present The Gapcoin developers
// Released under the GNU General Public License v3, http://www.gnu.org/licenses/

#ifndef __POWPROCESSOR_H__
#define __POWPROCESSOR_H__
#include <gmp.h>
#include "PoW.h"

class PoWProcessor {
public:
	PoWProcessor() { }
	virtual ~PoWProcessor() { }
	/** Should process a given PoW (e.g validate it and so on) should return whether to continue calculating or not */
	virtual bool process(PoW *pow) = 0;
};
#endif /* __POWPROCESSOR_H__ */
