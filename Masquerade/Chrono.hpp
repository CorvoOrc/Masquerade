#pragma once
#include <ctime>
#include "Event.hpp"

class IChrono {
	VIRTUAL_EVENT(Struck)
public:
	virtual void Tick() = 0;
	virtual long long GetStamp() = 0;
	virtual long long GetDiffS(long long ts) = 0;
};

class ChronoEventArgs :public IEventArgs {
public:
	ChronoEventArgs(IChrono* chrono) {
		_chrono = chrono;
	}
public:
	IChrono* _chrono;
};

class Chrono : public IChrono {
	EVENT(Struck)
public:
	Chrono() {
		start = time(0);
		Struck = new Event();
	}
	~Chrono() {
		delete Struck;
	}
	void Tick() {
		auto cur = clock();
		worked += cur - savepoint;
		bool isStriking = savepoint < cur;
		savepoint = cur;

		if (isStriking)
			Struck->Invoke(new ChronoEventArgs(this));
	}
	long long GetStamp() {
		return start * CLOCKS_PER_SEC + worked;
	}
	long long GetDiffS(long long ts) {
		return (GetStamp() - ts) / CLOCKS_PER_SEC;
	}
private:
	long long worked = 0;
	time_t start;
	time_t savepoint;
};