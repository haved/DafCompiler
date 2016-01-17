#ifndef MAIN_DAF_HEADER_GUARD
#define MAIN_DAF_HEADER_GUARD

#include <vector>
#include <iostream>
#include <memory>

#include "Counter.h"

class MainClass : public CounterListener {
	public:
		MainClass();
		void AddCounter(std::shared_ptr<Counter> counter);
		void RemoveCounter(usize id);
		void CountAll(int c);
		void OnCounterFinish(usize id);
	private:
		std::vector<std::shared_ptr<Counter>> m_counters;
};

#endif