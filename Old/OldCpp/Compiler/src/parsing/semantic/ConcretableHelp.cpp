#include "parsing/semantic/ConcretableHelp.hpp"

allConcrete allConcrete::operator !() const {
	allConcrete retur = *this;
	retur.reversed = !retur.reversed;
	return retur;
}

allConcrete allConcrete::operator <<(ConcretableState state) const {
	allConcrete retur = *this;
	retur <<= state;
	return retur;
}

allConcrete& allConcrete::operator <<=(ConcretableState state) {
	all &= state == ConcretableState::CONCRETE;
	return *this;
}

allConcrete::operator bool() const {
	return all != reversed;
}

anyLost anyLost::operator <<(ConcretableState state) const {
	anyLost retur = *this;
	retur <<= state;
	return retur;
}

anyLost& anyLost::operator <<=(ConcretableState state) {
	any |= state == ConcretableState::LOST_CAUSE;
	return *this;
}

anyLost::operator bool() const {
	return any;
}

bool tryLater(ConcretableState state) {
	return state == ConcretableState::TRY_LATER;
}
