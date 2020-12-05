#pragma once

//STD
#include <type_traits>

namespace enki {
//Should be the default choice, particularly in non-generic or end-user code
//Ensures at compile-time that a move is performed, and not a copy
template <typename T>
decltype(auto) move(T&& t)
{
	constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;
	static_assert(!is_const,
		"T is const and therefore can only be copied, not moved. Use std::move instead.");
	return static_cast<typename remove_reference_t<T>&&>(t);
};
}