#pragma once

//SELF
#include "Enki/Hash.hpp"

namespace enki
{

class Font
{
public:
	Font(HashedID type)
		: type(type)
	{
	}

	Font(const Font&) = delete;
	Font& operator=(const Font&) = delete;
	Font(Font&&) noexcept = delete;
	Font& operator=(Font&&) noexcept = delete;
	virtual ~Font() noexcept = default;

	virtual bool loadFromFile(const std::string& file) = 0;
	
	[[nodiscard]] HashedID getType() const { return type; }

	template <typename T>
	[[nodiscard]] bool is()
	{
		static_assert(std::is_base_of_v<Font, T>, "This type is not derived from enki::Font");
		return T::type == type;
	}

	template <typename T>
	[[nodiscard]] T* as()
	{
		static_assert(std::is_base_of_v<Font, T>, "This type is not derived from enki::Font");
		return T::type == type ? static_cast<T*>(this) : nullptr;
	}

	template <typename T>
	[[nodiscard]] const T* as() const
	{
		static_assert(std::is_base_of_v<Font, T>, "This type is not derived from enki::Font");
		return T::type == type ? static_cast<const T*>(this) : nullptr;
	}

protected:
	HashedID type;
};

}