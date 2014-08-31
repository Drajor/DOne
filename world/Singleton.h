#pragma once

template <typename S>
class Singleton {
public:
	static inline S& getInstance() {
		static S instance;
		return instance;
	}
protected:
	Singleton() {};
	virtual ~Singleton() {};
	Singleton(Singleton const&);
	void operator=(Singleton const&);
};