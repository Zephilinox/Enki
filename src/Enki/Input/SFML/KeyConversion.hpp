#pragma once

//SELF
#include "Enki/Input/Keys.hpp"

namespace enki
{
Keyboard::Key keyConversionFromSFML(int sfml_key);
inline int keyConversionToSFML(Keyboard::Key)
{
	return -1;
};

Mouse::Button buttonConversionFromSFML(int sfml_button);
inline int buttonConversionToSFML(Mouse::Button)
{
	return -1;
};
}	 // namespace enki