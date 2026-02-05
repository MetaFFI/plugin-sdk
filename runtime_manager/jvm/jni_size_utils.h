#pragma once

#include <limits>
#include <stdexcept>
#include <type_traits>

#ifdef _DEBUG
#undef _DEBUG
#include <jni.h>
#define _DEBUG
#else
#include <jni.h>
#endif

#ifdef _WIN32
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif

template <typename T>
inline jsize to_jsize(T value)
{
	static_assert(std::is_integral_v<T>, "to_jsize expects integral type");

	if constexpr (std::is_signed_v<T>)
	{
		if(value < 0)
		{
			throw std::out_of_range("negative size cannot be converted to jsize");
		}
	}

	using UnsignedT = std::make_unsigned_t<T>;
	if(static_cast<UnsignedT>(value) > static_cast<UnsignedT>((std::numeric_limits<jsize>::max)()))
	{
		throw std::out_of_range("size exceeds jsize range");
	}

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4244 4267)
#endif
	return static_cast<jsize>(value);
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}
