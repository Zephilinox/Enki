#include "KeyConversion.hpp"

//LIBS
#include <SFML/Window.hpp>

namespace enki
{
Keyboard::Key keyConversionFromSFML(int sfml_key)
{
	switch (sfml_key)
	{
		// clang-format off
		case sf::Keyboard::Key::Unknown:	return Keyboard::Key::Unknown;
		case sf::Keyboard::Key::A:			return Keyboard::Key::A;
		case sf::Keyboard::Key::B:			return Keyboard::Key::B;
		case sf::Keyboard::Key::C:			return Keyboard::Key::C;
		case sf::Keyboard::Key::D:			return Keyboard::Key::D;
		case sf::Keyboard::Key::E:			return Keyboard::Key::E;
		case sf::Keyboard::Key::F:			return Keyboard::Key::F;
		case sf::Keyboard::Key::G:			return Keyboard::Key::G;
		case sf::Keyboard::Key::H:			return Keyboard::Key::H;
		case sf::Keyboard::Key::I:			return Keyboard::Key::I;
		case sf::Keyboard::Key::J:			return Keyboard::Key::J;
		case sf::Keyboard::Key::K:			return Keyboard::Key::K;
		case sf::Keyboard::Key::L:			return Keyboard::Key::L;
		case sf::Keyboard::Key::M:			return Keyboard::Key::M;
		case sf::Keyboard::Key::N:			return Keyboard::Key::N;
		case sf::Keyboard::Key::O:			return Keyboard::Key::O;
		case sf::Keyboard::Key::P:			return Keyboard::Key::P;
		case sf::Keyboard::Key::Q:			return Keyboard::Key::Q;
		case sf::Keyboard::Key::R:			return Keyboard::Key::R;
		case sf::Keyboard::Key::S:			return Keyboard::Key::S;
		case sf::Keyboard::Key::T:			return Keyboard::Key::T;
		case sf::Keyboard::Key::U:			return Keyboard::Key::U;
		case sf::Keyboard::Key::V:			return Keyboard::Key::V;
		case sf::Keyboard::Key::W:			return Keyboard::Key::W;
		case sf::Keyboard::Key::X:			return Keyboard::Key::X;
		case sf::Keyboard::Key::Y:			return Keyboard::Key::Y;
		case sf::Keyboard::Key::Z:			return Keyboard::Key::Z;
		case sf::Keyboard::Key::Num0:		return Keyboard::Key::Num0;
		case sf::Keyboard::Key::Num1:		return Keyboard::Key::Num1;
		case sf::Keyboard::Key::Num2:		return Keyboard::Key::Num2;
		case sf::Keyboard::Key::Num3:		return Keyboard::Key::Num3;
		case sf::Keyboard::Key::Num4:		return Keyboard::Key::Num4;
		case sf::Keyboard::Key::Num5:		return Keyboard::Key::Num5;
		case sf::Keyboard::Key::Num6:		return Keyboard::Key::Num6;
		case sf::Keyboard::Key::Num7:		return Keyboard::Key::Num7;
		case sf::Keyboard::Key::Num8:		return Keyboard::Key::Num8;
		case sf::Keyboard::Key::Num9:		return Keyboard::Key::Num9;

		case sf::Keyboard::Key::Escape:		return Keyboard::Key::Escape;
		case sf::Keyboard::Key::LControl:	return Keyboard::Key::Control;
		case sf::Keyboard::Key::LShift:		return Keyboard::Key::Shift;
		case sf::Keyboard::Key::LAlt:		return Keyboard::Key::Alt;
		case sf::Keyboard::Key::LSystem:	return Keyboard::Key::System;
		case sf::Keyboard::Key::RControl:	return Keyboard::Key::Control;
		case sf::Keyboard::Key::RShift:		return Keyboard::Key::Shift;
		case sf::Keyboard::Key::RAlt:		return Keyboard::Key::Alt;
		case sf::Keyboard::Key::RSystem:	return Keyboard::Key::System;

		case sf::Keyboard::Key::Menu:		return Keyboard::Key::Menu;
		case sf::Keyboard::Key::LBracket:	return Keyboard::Key::LeftBracket;
		case sf::Keyboard::Key::RBracket:	return Keyboard::Key::RightBracket;
		case sf::Keyboard::Key::Semicolon:	return Keyboard::Key::Semicolon;
		case sf::Keyboard::Key::Comma:		return Keyboard::Key::Comma;
		case sf::Keyboard::Key::Period:		return Keyboard::Key::Period;
		case sf::Keyboard::Key::Quote:		return Keyboard::Key::Quote;
		case sf::Keyboard::Key::Slash:		return Keyboard::Key::Slash;
		case sf::Keyboard::Key::Backslash:	return Keyboard::Key::Backslash;
		case sf::Keyboard::Key::Tilde:		return static_cast<Keyboard::Key>(sfml_key);
		case sf::Keyboard::Key::Equal:		return static_cast<Keyboard::Key>(sfml_key);
		case sf::Keyboard::Key::Dash:		return Keyboard::Key::Dash;
		case sf::Keyboard::Key::Space:		return Keyboard::Key::Space;
		case sf::Keyboard::Key::Enter:		return Keyboard::Key::Enter;
		case sf::Keyboard::Key::Backspace:	return Keyboard::Key::Backspace;
		case sf::Keyboard::Key::Tab:		return Keyboard::Key::Tab;
		case sf::Keyboard::Key::PageUp:		return Keyboard::Key::PageUp;
		case sf::Keyboard::Key::PageDown:	return Keyboard::Key::PageDown;
		case sf::Keyboard::Key::End:		return Keyboard::Key::End;
		case sf::Keyboard::Key::Home:		return Keyboard::Key::Home;
		case sf::Keyboard::Key::Insert:		return Keyboard::Key::Insert;
		case sf::Keyboard::Key::Delete:		return Keyboard::Key::Delete;
		case sf::Keyboard::Key::Add:		return Keyboard::Key::Plus;
		case sf::Keyboard::Key::Subtract:	return Keyboard::Key::Minus;
		case sf::Keyboard::Key::Multiply:	return Keyboard::Key::Multiply;
		case sf::Keyboard::Key::Divide:		return Keyboard::Key::Divide;
		case sf::Keyboard::Key::Left:		return Keyboard::Key::Left;
		case sf::Keyboard::Key::Right:		return Keyboard::Key::Right;
		case sf::Keyboard::Key::Up:			return Keyboard::Key::Up;
		case sf::Keyboard::Key::Down:		return Keyboard::Key::Down;
		case sf::Keyboard::Key::Numpad0:	return Keyboard::Key::Numpad0;
		case sf::Keyboard::Key::Numpad1:	return Keyboard::Key::Numpad1;
		case sf::Keyboard::Key::Numpad2:	return Keyboard::Key::Numpad2;
		case sf::Keyboard::Key::Numpad3:	return Keyboard::Key::Numpad3;
		case sf::Keyboard::Key::Numpad4:	return Keyboard::Key::Numpad4;
		case sf::Keyboard::Key::Numpad5:	return Keyboard::Key::Numpad5;
		case sf::Keyboard::Key::Numpad6:	return Keyboard::Key::Numpad6;
		case sf::Keyboard::Key::Numpad7:	return Keyboard::Key::Numpad7;
		case sf::Keyboard::Key::Numpad8:	return Keyboard::Key::Numpad8;
		case sf::Keyboard::Key::Numpad9:	return Keyboard::Key::Numpad9;
		case sf::Keyboard::Key::F1:			return Keyboard::Key::F1;
		case sf::Keyboard::Key::F2:			return Keyboard::Key::F2;
		case sf::Keyboard::Key::F3:			return Keyboard::Key::F3;
		case sf::Keyboard::Key::F4:			return Keyboard::Key::F4;
		case sf::Keyboard::Key::F5:			return Keyboard::Key::F5;
		case sf::Keyboard::Key::F6:			return Keyboard::Key::F6;
		case sf::Keyboard::Key::F7:			return Keyboard::Key::F7;
		case sf::Keyboard::Key::F8:			return Keyboard::Key::F8;
		case sf::Keyboard::Key::F9:			return Keyboard::Key::F9;
		case sf::Keyboard::Key::F10:		return Keyboard::Key::F10;
		case sf::Keyboard::Key::F11:		return Keyboard::Key::F11;
		case sf::Keyboard::Key::F12:		return Keyboard::Key::F12;
		case sf::Keyboard::Key::F13:		return static_cast<Keyboard::Key>(sfml_key);
		case sf::Keyboard::Key::F14:		return static_cast<Keyboard::Key>(sfml_key);
		case sf::Keyboard::Key::F15:		return static_cast<Keyboard::Key>(sfml_key);
		case sf::Keyboard::Key::Pause:		return Keyboard::Key::Pause;
		case sf::Keyboard::Key::KeyCount:	return Keyboard::Key::KeyCount;
		default:							return Keyboard::Key::Unknown;
		//case sf::Keyboard::Key::Capslock:	return Keyboard::Key::Capslock;
		//case sf::Keyboard::Key::Function:	return Keyboard::Key::Function;
		//case sf::Keyboard::Key::Hash:		return Keyboard::Key::Hash;
		//case sf::Keyboard::Key::PrintScreen:	return Keyboard::Key::PrintScreen;
		// clang-format on
	}
}

Mouse::Button buttonConversionFromSFML(int sfml_button)
{
	switch (sfml_button)
	{
		// clang-format off
		case sf::Mouse::Button::Left:		return Mouse::Button::Left;
		case sf::Mouse::Button::Right:		return Mouse::Button::Right;
		case sf::Mouse::Button::Middle:		return Mouse::Button::Middle;
		case sf::Mouse::Button::XButton1:	return Mouse::Button::Button4;
		case sf::Mouse::Button::XButton2:	return Mouse::Button::Button5;
		default:							return Mouse::Button::Unknown;
		// clang-format on
	}
}

}