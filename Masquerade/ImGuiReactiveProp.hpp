#pragma once
#include "Event.hpp"

template <typename T>
class ReactiveProp {
	EVENT(Changed)
public:
	ReactiveProp(T value) {
		Changed = new Event();
		_value = value;
		_last_setted_value = value;
	}
	virtual void Set(T value) = 0;
	virtual T Get() = 0;
	virtual T& GetRef() = 0;
	virtual void ForceCheck() = 0;
protected:
	void Change() {
		Changed->Invoke(new RectiveEventArgs(this));
	}
protected:
	T _value;
	T _last_setted_value;
};

template <typename T>
class RectiveEventArgs : public IEventArgs {
public:
	RectiveEventArgs(ReactiveProp<T>* prop) {
		_prop = prop;
	}
public:
	ReactiveProp<T>* _prop;
};

class ReactiveFloat : public ReactiveProp<float> {
public :
	ReactiveFloat(float value = 0.0) : ReactiveProp(value) { }
	void Set(float value) {
		_value = value;
		_last_setted_value = _value;
		Change();
	}
	float Get() {
		return _value;
	}
	float& GetRef() {
		return _value;
	}
	void ForceCheck() {
		if (abs(_last_setted_value - _value) > FLT_EPSILON)
			Set(_value);
	}
};