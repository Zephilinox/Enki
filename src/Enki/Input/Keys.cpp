#include "Keys.hpp"

namespace enki
{
std::string Keyboard::keyToString(const Key key)
{
	switch (key)
	{
		// clang-format off
		case Key::Unknown:		return "Unknown";
		case Key::A:			return "A";
		case Key::B:			return "B";
		case Key::C:			return "C";
		case Key::D:			return "D";
		case Key::E:			return "E";
		case Key::F:			return "F";
		case Key::G:			return "G";
		case Key::H:			return "H";
		case Key::I:			return "I";
		case Key::J:			return "J";
		case Key::K:			return "K";
		case Key::L:			return "L";
		case Key::M:			return "M";
		case Key::N:			return "N";
		case Key::O:			return "O";
		case Key::P:			return "P";
		case Key::Q:			return "Q";
		case Key::R:			return "R";
		case Key::S:			return "S";
		case Key::T:			return "T";
		case Key::U:			return "U";
		case Key::V:			return "V";
		case Key::W:			return "W";
		case Key::X:			return "X";
		case Key::Y:			return "Y";
		case Key::Z:			return "Z";
		case Key::Num0:			return "0";
		case Key::Num1:			return "1";
		case Key::Num2:			return "2";
		case Key::Num3:			return "3";
		case Key::Num4:			return "4";
		case Key::Num5:			return "5";
		case Key::Num6:			return "6";
		case Key::Num7:			return "7";
		case Key::Num8:			return "8";
		case Key::Num9:			return "9";
		case Key::Escape:		return "Escape";
		case Key::Tab:			return "Tab";
		case Key::Capslock:		return "Capslock";
		case Key::Shift:		return "Shift";
		case Key::Control:		return "Control";
		case Key::Alt:			return "Alt";
		case Key::Command:		return "Command";
		case Key::Function:		return "Function";
		case Key::Menu:			return "Menu";
		case Key::Enter:		return "Enter";
		case Key::Backspace:	return "Backspace";
		case Key::Comma:		return "Comma";
		case Key::Period:		return "Period";
		case Key::Slash:		return "Slash";
		case Key::Backslash:	return "Backslash";
		case Key::Semicolon:	return "Semicolon";
		case Key::Quote:		return "Quote";
		case Key::Hash:			return "Hash";
		case Key::LeftBracket:	return "LeftBracket";
		case Key::RightBracket:	return "RightBracket";
		case Key::Dash:			return "Dash";
		case Key::Space:		return "Space";
		case Key::PageUp:		return "PageUp";
		case Key::PageDown:		return "PageDown";
		case Key::Home:			return "Home";
		case Key::End:			return "End";
		case Key::Delete:		return "Delete";
		case Key::Insert:		return "Insert";
		case Key::Minus:		return "Minus";
		case Key::Plus:			return "Plus";
		case Key::Divide:		return "Divide";
		case Key::Multiply:		return "Multiply";
		case Key::Numpad0:		return "Numpad0";
		case Key::Numpad1:		return "Numpad1";
		case Key::Numpad2:		return "Numpad2";
		case Key::Numpad3:		return "Numpad3";
		case Key::Numpad4:		return "Numpad4";
		case Key::Numpad5:		return "Numpad5";
		case Key::Numpad6:		return "Numpad6";
		case Key::Numpad7:		return "Numpad7";
		case Key::Numpad8:		return "Numpad8";
		case Key::Numpad9:		return "Numpad9";
		case Key::Up:			return "Up";
		case Key::Down:			return "Down";
		case Key::Left:			return "Left";
		case Key::Right:		return "Right";
		case Key::F1:			return "F1";
		case Key::F2:			return "F2";
		case Key::F3:			return "F3";
		case Key::F4:			return "F4";
		case Key::F5:			return "F5";
		case Key::F6:			return "F6";
		case Key::F7:			return "F7";
		case Key::F8:			return "F8";
		case Key::F9:			return "F9";
		case Key::F10:			return "F10";
		case Key::F11:			return "F11";
		case Key::F12:			return "F12";
		case Key::Pause:		return "Pause";
		case Key::PrintScreen:	return "PrintScreen";
		default:				return std::string("Unknown ") + std::to_string(static_cast<int>(key));
			// clang-format on
	}
}

std::string Mouse::buttonToString(const Button button)
{
	switch (button)
	{
		// clang-format off
		case Button::Unknown:	return "Unknown";
		case Button::Left:		return "Left";
		case Button::Right:		return "Right";
		case Button::Middle:	return "Middle";
		case Button::Button4:	return "Button4";
		case Button::Button5:	return "Button5";
		default:				return std::string("Unknown ") + std::to_string(static_cast<int>(button));
			// clang-format on
	}
}
}	 // namespace enki