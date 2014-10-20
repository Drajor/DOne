#pragma once

#include "Constants.h"

class Time {
public:
	template< typename T>
	static inline T now() {
		return std::chrono::duration_cast<T>(std::chrono::system_clock::now().time_since_epoch());
	}

	static inline int64 nowMilliseconds() { return now<std::chrono::milliseconds>().count(); }
	static inline int64 nowSeconds() { return now<std::chrono::seconds>().count(); }
	static inline int64 nowMinutes() { return now<std::chrono::minutes>().count(); }
};