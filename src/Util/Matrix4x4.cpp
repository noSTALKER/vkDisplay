#include "Matrix4x4.h"
#include <memory>

namespace vkDisplay
{

Matrix4x4::Matrix4x4(float* data)
{
	memcpy(mData, data, 16 * sizeof(float));
}

Matrix4x4::Matrix4x4(const Matrix4x4& rhs) 
{
	memcpy(mData, rhs.mData, 16 * sizeof(float));
}

Matrix4x4&
Matrix4x4::operator=(const Matrix4x4& rhs)
{
	if (this != &rhs) {
		memcpy(mData, rhs.mData, 16 * sizeof(float));
	}
	return *this;
}

void 
Matrix4x4::setIdentity()
{
	memset(mData, 0, 16 * sizeof(float));
	mIndex.m11 = mIndex.m22 = mIndex.m33 = mIndex.m44 = 1;

}

void 
Matrix4x4::setView(const Vector3 & eye, const Vector3 & dir, Vector3 & up)
{
	setIdentity();
}

void 
Matrix4x4::setProjection(float near, float far, float width, float aspect)
{
	setIdentity();
}

void 
Matrix4x4::setModel(const Vector3& position, const Vector3& scale, const Vector3& axis, float angle)
{
	setIdentity();

	//set the translation
	mIndex.m14 = position.x();
	mIndex.m24 = position.y();
	mIndex.m34 = position.z();

	//set the scale 
	mIndex.m11 = scale.x();
	mIndex.m22 = scale.y();
	mIndex.m33 = scale.z();

	//set the rotation
}

Matrix4x4 
Matrix4x4::operator * (const Matrix4x4& rhs) const
{
	float m11 = mIndex.m11 * rhs.mIndex.m11 + mIndex.m12 * rhs.mIndex.m21 + mIndex.m13 * rhs.mIndex.m31 + mIndex.m14 * rhs.mIndex.m41;
	float m12 = mIndex.m11 * rhs.mIndex.m12 + mIndex.m12 * rhs.mIndex.m22 + mIndex.m13 * rhs.mIndex.m32 + mIndex.m14 * rhs.mIndex.m42;
	float m13 = mIndex.m11 * rhs.mIndex.m13 + mIndex.m12 * rhs.mIndex.m23 + mIndex.m13 * rhs.mIndex.m33 + mIndex.m14 * rhs.mIndex.m43;
	float m14 = mIndex.m11 * rhs.mIndex.m14 + mIndex.m12 * rhs.mIndex.m24 + mIndex.m13 * rhs.mIndex.m34 + mIndex.m14 * rhs.mIndex.m44;

	float m21 = mIndex.m21 * rhs.mIndex.m11 + mIndex.m22 * rhs.mIndex.m21 + mIndex.m23 * rhs.mIndex.m31 + mIndex.m24 * rhs.mIndex.m41;
	float m22 = mIndex.m21 * rhs.mIndex.m12 + mIndex.m22 * rhs.mIndex.m22 + mIndex.m23 * rhs.mIndex.m32 + mIndex.m24 * rhs.mIndex.m42;
	float m23 = mIndex.m21 * rhs.mIndex.m13 + mIndex.m22 * rhs.mIndex.m23 + mIndex.m23 * rhs.mIndex.m33 + mIndex.m24 * rhs.mIndex.m43;
	float m24 = mIndex.m21 * rhs.mIndex.m14 + mIndex.m22 * rhs.mIndex.m24 + mIndex.m23 * rhs.mIndex.m34 + mIndex.m24 * rhs.mIndex.m44;

	float m31 = mIndex.m31 * rhs.mIndex.m11 + mIndex.m32 * rhs.mIndex.m21 + mIndex.m33 * rhs.mIndex.m31 + mIndex.m34 * rhs.mIndex.m41;
	float m32 = mIndex.m31 * rhs.mIndex.m12 + mIndex.m32 * rhs.mIndex.m22 + mIndex.m33 * rhs.mIndex.m32 + mIndex.m34 * rhs.mIndex.m42;
	float m33 = mIndex.m31 * rhs.mIndex.m13 + mIndex.m32 * rhs.mIndex.m23 + mIndex.m33 * rhs.mIndex.m33 + mIndex.m34 * rhs.mIndex.m43;
	float m34 = mIndex.m31 * rhs.mIndex.m14 + mIndex.m32 * rhs.mIndex.m24 + mIndex.m33 * rhs.mIndex.m34 + mIndex.m34 * rhs.mIndex.m44;

	float m41 = mIndex.m41 * rhs.mIndex.m11 + mIndex.m42 * rhs.mIndex.m21 + mIndex.m43 * rhs.mIndex.m31 + mIndex.m44 * rhs.mIndex.m41;
	float m42 = mIndex.m41 * rhs.mIndex.m12 + mIndex.m42 * rhs.mIndex.m22 + mIndex.m43 * rhs.mIndex.m32 + mIndex.m44 * rhs.mIndex.m42;
	float m43 = mIndex.m41 * rhs.mIndex.m13 + mIndex.m42 * rhs.mIndex.m23 + mIndex.m43 * rhs.mIndex.m33 + mIndex.m44 * rhs.mIndex.m43;
	float m44 = mIndex.m41 * rhs.mIndex.m14 + mIndex.m42 * rhs.mIndex.m24 + mIndex.m43 * rhs.mIndex.m34 + mIndex.m44 * rhs.mIndex.m44;

	return Matrix4x4(m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
}

Vector4
Matrix4x4::operator * (const Vector4& rhs) const
{
	Vector4 x1 = mColumn.c1 * rhs;
	Vector4 x2 = mColumn.c2 * rhs;
	Vector4 x3 = mColumn.c3 * rhs;
	Vector4 x4 = mColumn.c4 * rhs;

	float x = x1.x() + x2.x() + x3.x() + x4.x();
	float y = x1.y() + x2.y() + x3.y() + x4.y();
	float z = x1.z() + x2.z() + x3.z() + x4.z();
	float w = x1.w() + x2.w() + x3.w() + x4.w();

	return Vector4(x, y, z, w);
}

}
