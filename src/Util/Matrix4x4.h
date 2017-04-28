#pragma once

#include "Vector4.h"
#include "Vector3.h"
#include "Export.h"

namespace vkDisplay
{
class UTIL_API alignas(16) Matrix4x4
{
public:
	Matrix4x4()
	{
		setIdentity();
	}

	Matrix4x4(float* data);

	Matrix4x4(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24, float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44)
	{
		mIndex.m11 = m11;
		mIndex.m12 = m12;
		mIndex.m13 = m13;
		mIndex.m14 = m14;
		mIndex.m21 = m21;
		mIndex.m22 = m22;
		mIndex.m23 = m23;
		mIndex.m24 = m24;
		mIndex.m31 = m31;
		mIndex.m32 = m32;
		mIndex.m33 = m33;
		mIndex.m34 = m34;
		mIndex.m41 = m41;
		mIndex.m42 = m42;
		mIndex.m43 = m43;
		mIndex.m44 = m44;
	}

	Matrix4x4(const Matrix4x4&);
	Matrix4x4& operator=(const Matrix4x4&);

	void setIdentity();
	void setView(const Vector3& eye, const Vector3& dir, Vector3& up);
	void setProjection(float near, float far, float width, float aspectRatio);
	void setModel(const Vector3& position, const Vector3& scale, const Vector3& axis, float angle);
	Matrix4x4 operator * (const Matrix4x4& rhs) const;
	Vector4 operator * (const Vector4& rhs) const;

private:
	union {
		struct {
			float m11, m21, m31, m41, m12, m22, m32, m42, m13, m23, m33, m43, m14, m24, m34, m44;
			} mIndex;
		struct {
			Vector4 c1, c2, c3, c4;
		} mColumn;
		float mData[16];
	};
};
}