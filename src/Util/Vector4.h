#pragma once

#include <immintrin.h>
#include "Export.h"

namespace vkDisplay
{
class UTIL_API alignas(16) Vector4
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

	float& x() {
		return data.x;	
	}

	float x() const {
		return data.x;
	}

	float& y() {
		return data.y;
	}

	float y() const {
		return data.y;
	}

	float& z() {
		return data.z;
	}

	float z() const {
		return data.z;
	}

	float& w() {
		return data.w;
	}

	float w() const {
		return data.w;
	}

	Vector4 operator+(const Vector4& rhs) const {
		return _mm_add_ps(value, rhs.value);
	}

	Vector4& operator+=(const Vector4& rhs) {
		value = _mm_add_ps(value, rhs.value);
		return *this;
	}

	Vector4 operator-(const Vector4& rhs) const {
		return _mm_sub_ps(value, rhs.value);
	}
	
	Vector4 operator*(const Vector4& rhs) const {
		return _mm_mul_ps(value, rhs.value);
	}

	Vector4 operator/(const Vector4& rhs) const {
		return _mm_div_ps(value, rhs.value);
	}

	float horizontalSum() const {
		float sum;
		__m128 hsum = _mm_hadd_ps(value, value);
		hsum = _mm_hadd_ps(hsum, hsum);
		_mm_store_ss(&sum, hsum);
		return sum;
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