#pragma once

#include "Concretable.hpp"

class allConcrete {
	bool reversed = false;
	bool all = true;
public:
	allConcrete operator !() const {
	    allConcrete retur = *this;
		retur.reversed = !retur.reversed;
		return retur;
	}

	allConcrete operator <<(ConcretableState state) const {
		allConcrete retur = *this;
	    retur <<= state;
		return retur;
	}

	allConcrete& operator <<=(ConcretableState state) {
		all &= state == ConcretableState::CONCRETE;
		return *this;
	}

	operator bool() const {
		return all != reversed;
	}
};

class anyLost {
	bool any=false;
public:
	anyLost operator <<(ConcretableState state) const {
		anyLost retur = *this;
		retur <<= state;
		return retur;
	}

	anyLost& operator <<=(ConcretableState state) {
		any |= state == ConcretableState::LOST_CAUSE;
		return *this;
	}

	operator bool() const {
		return any;
	}

};

bool tryLater(ConcretableState state) {
	return state == ConcretableState::TRY_LATER;
}
