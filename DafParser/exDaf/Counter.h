#ifndef COUNTER_DAF_HEADER_GUARD
#define COUNTER_DAF_HEADER_GUARD

#include <stdint.h>
#include <cstddef>

#include <string>

#define usize unsigned int

class CounterListener {
public:
	virtual void OnCounterFinish(usize id)=0;
};

class Counter {
public:
	Counter(int max, const std::string &name);
	void SetFinishListener(CounterListener* listener);
	void SetId(usize id);
	void Count(int n);
	std::string* GetName();
private:
	int m_max;
	int m_count;
	usize m_id;
	CounterListener* m_listener;
	std::string m_name;
};
#endif
