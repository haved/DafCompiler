#pragma once

#include "Concretable.hpp"

class allConcrete {
	bool reversed = false;
	bool all = true;
public:
	allConcrete operator !() const;
	allConcrete operator <<(ConcretableState state) const;
	allConcrete& operator <<=(ConcretableState state);
	operator bool() const;
};

class anyLost {
	bool any=false;
public:
	anyLost operator <<(ConcretableState state) const;
	anyLost& operator <<=(ConcretableState state);
	operator bool() const;

};

bool tryLater(ConcretableState state);
