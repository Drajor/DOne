#pragma once

#include <stdint.h>
#include <memory>

template <class T>
using SharedPtr = std::shared_ptr<T>;

//template std::shared_ptr<T> SharedPtr<T>;
//typedef std::shared_ptr<T> SharedPtr<T>;

typedef uint8_t byte;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#include <string>
#include <sstream>
typedef std::string String;
typedef std::stringstream StringStream;

template<typename T>
String toString(T pValue) { return std::to_string(pValue); }

struct Colour3 {
	u8 mRed = 0;
	u8 mGreen = 0;
	u8 mBlue = 0;
};

struct Colour4 {
	u8 mRed = 0;
	u8 mGreen = 0;
	u8 mBlue = 0;
	u8 mTint = 0;
};
