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
		retur.all &= state == ConcretableState::CONCRETE;
		return retur;
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
		retur.any |= state == ConcretableState::LOST_CAUSE;
		return retur;
	}

	operator bool() const {
		return any;
	}
};
