#pragma once

#pragma once

#include "Export.h"

namespace vkDisplay
{
	class UTIL_API Vector3
	{
	public:
		Vector3(float x, float y, float z)
		{
			data.x = x;
			data.y = y;
			data.z = z;
		}

		Vector3(const Vector3& rhs)
		{
			data.x = rhs.data.x;
			data.y = rhs.data.y;
			data.z = rhs.data.z;
		}

		Vector3& operator=(const Vector3& rhs)
		{
			if (this != &rhs) {
				data.x = rhs.data.x;
				data.y = rhs.data.y;
				data.z = rhs.data.z;
			}
			return *this;
		}

		float x() const&
		{
			return data.x;
		}

		float y() const&
		{
			return data.y;
		}

		float z() const&
		{
			return data.z;
		}

		float& x() &
		{
			return data.x;
		}

		float& y() &
		{
			return data.y;
		}

		float& z() &
		{
			return data.z;
		}

	private:

		union {
			struct {
				float x, y, z;
			} data;
		};
	};
}