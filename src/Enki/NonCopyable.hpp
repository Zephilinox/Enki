#pragma once

//todo: what was I thinking when I made this?

namespace enki
{
/*
 * Deleting the copy constructor/assignment implicitly deletes the move constructor/assignment
 * Declaring them to be = default has no affect on derived classes
 * A derived class that requires move semantics must implement a move constructor/assignment manually
 */
class NonCopyableAndNonMovable
{
public:
	inline constexpr NonCopyableAndNonMovable(const NonCopyableAndNonMovable&) noexcept = delete;
	inline constexpr NonCopyableAndNonMovable& operator=(const NonCopyableAndNonMovable&) noexcept = delete;

protected:
	inline constexpr NonCopyableAndNonMovable() noexcept = default;
	inline ~NonCopyableAndNonMovable() noexcept = default;
};
}