#include "Counter.h"

Counter::Counter(int max, const std::string &name) : m_max(max), m_name(name), m_listener(NULL), m_id(0), m_count(0) {}

void Counter::SetFinishListener(CounterListener* listener) {
	m_listener = listener;
}

void Counter::SetId(usize id) {
	m_id = id;
}

void Counter::Count(int n) {
	m_count+=n;
	if(m_count > m_max & m_listener!=NULL)
		m_listener->OnCounterFinish(m_id);
}

std::string* Counter::GetName() {
	return &m_name;
}