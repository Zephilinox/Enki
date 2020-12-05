#pragma once

//SELF
#include "Enki/Graphics/Font.hpp"

namespace enki
{

class FontNone final : public Font
{
public:
	FontNone();

	bool loadFromFile(const std::string& file) final;
	
	static constexpr HashedID type = hash_constexpr("None");

private:
};

}	 // namespace enki