#include "FontNone.hpp"

namespace enki
{

FontNone::FontNone()
	: Font(type)
{
	
}

bool FontNone::loadFromFile(const std::string&)
{
	return true;
}

}	 // namespace enki