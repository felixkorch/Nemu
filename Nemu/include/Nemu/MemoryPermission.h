// ---------------------------------------------------------------------* C++ *-
// MemoryPermission.h
//
// -----------------------------------------------------------------------------

#pragma once

namespace nemu
{
	enum MemoryPermissionFlag {
		C = 0,
		R = 1,
		W = 2,
		RW = 3,
	};

	template <class T>
	class MemoryPermission {
		const bool readPermission;
		const bool writePermission;
		T *data;

	    public:
		MemoryPermission(MemoryPermissionFlag flag, T *data)
			: readPermission((flag & 1) != 0),
			  writePermission((flag & 2) != 0), data(data)
		{}

		operator const T() const
		{
			if (readPermission)
				return *data;
			return T();
		}

		MemoryPermission<T> &operator=(const T &value)
		{
			if (writePermission)
				*data = value;
			return *this;
		}

		MemoryPermission<T> &operator=(const MemoryPermission<T> &other)
		{
			if (writePermission)
				*data = other;
			return *this;
		}
	};
} // namespace nemu