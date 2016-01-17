#include "Main.h"

MainClass::MainClass() : m_counters({}) {}

void MainClass::AddCounter(std::shared_ptr<Counter> counter) {
	counter->SetFinishListener(this);
	counter->SetId(m_counters.size());
	m_counters.push_back(counter);
}

void MainClass::RemoveCounter(usize id) {
	m_counters.back()->SetId(id);
	m_counters[id] = m_counters.back();
	m_counters.pop_back();
	if(m_counters.empty())
		std::cout << "No counters are left!" << std::endl;
}

void MainClass::CountAll(int c) {
	for(usize i = 0; i < m_counters.size(); i++)
		m_counters[i]->Count(c);
}

void MainClass::OnCounterFinish(usize id) {
	std::cout << "The counter " << id << " named \"" << m_counters[id]->GetName()[0] << "\" finished!" << std::endl;
	RemoveCounter(id);
}

void nothing(Counter* c){}

int main() {
	MainClass mainClass;
	auto myCounter = std::shared_ptr<Counter>(new Counter(10, "Bolle"));
	auto secondCounter = new Counter(5, "Balle");
	mainClass.AddCounter(myCounter);
	mainClass.AddCounter(std::shared_ptr<Counter>(secondCounter));
	mainClass.AddCounter(std::make_shared<Counter>(12, "Bulle"));
	Counter counter(7, "Belle");
	mainClass.AddCounter(std::shared_ptr<Counter>(&counter, nothing));
	for(int i = 0; i < 20; i++)
		mainClass.CountAll(1);
	return 0;
}
