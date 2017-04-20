#pragma once

#include <immintrin.h>

namespace vkDisplay
{
class alignas(16) Vector4
{
public:
	Vector4(float x, float y, float z, float w)
	{
		value = _mm_set_ps(x, y, z, w);
	}

	Vector4(const Vector4& rhs)
	{
		value = rhs.value;
	}

	Vector4& operator=(const Vector4& rhs)
	{
		if (this != &rhs) {
			value = rhs.value;
		}
		return *this;
	}

	Vector4 operator+(const Vector4& rhs) {
		return _mm_add_ps(value, rhs.value);
	}

	Vector4& operator+=(const Vector4& rhs) {
		value = _mm_add_ps(value, rhs.value);
		return *this;
	}

	Vector4 operator-(const Vector4& rhs) {
		return _mm_sub_ps(value, rhs.value);
	}
	
	Vector4 operator*(const Vector4& rhs) {
		return _mm_mul_ps(value, rhs.value);
	}

	Vector4 operator/(const Vector4& rhs) {
		return _mm_div_ps(value, rhs.value);
	}

private:

	Vector4(const __m128& value)
		: value (value)
	{
	}

	union {
		struct {
			float x, y, z, w;
		} data;
		__m128 value;
	};
};
}