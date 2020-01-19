#pragma once

//STD
#include <variant>

//SELF
#include "Keys.hpp"

namespace enki
{
//http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0051r3.pdf
//https://arne-mertz.de/2018/05/overload-build-a-variant-visitor-on-the-fly/
//https://www.bfilipek.com/2018/09/visit-variants.html
//https://pabloariasal.github.io/2018/06/26/std-variant/
//https://bitbashing.io/std-visit.html
// clang-format off
template<class... Ts> struct overload : Ts... { using Ts::operator()...; };
template<class... Ts> overload(Ts...)->overload<Ts...>;
// clang-format on

struct EventUnknown
{
	//May store some information about the unknown event
	//For instance, when using SFML 'data' will contain the SFML event ID
	int data = -1;
};

struct EventQuit
{
};

struct EventResize
{
	unsigned int width = 0;
	unsigned int height = 0;
};

struct EventFocus
{
	bool focused = false;
};

struct EventKey
{
	Keyboard::Key key = Keyboard::Key::Unknown;
	bool down = false;
	unsigned int native_key = 0;
};

struct EventMouseMove
{
	int x = 0;
	int y = 0;
};

struct EventMouseButton
{
	Mouse::Button button;
	bool down = false;
};

struct EventMouseWheel
{
	float horizontal = 0;
	float vertical = 0;
};

struct EventMouseWindow
{
	bool inside = false;
};

struct EventCharacter
{
	unsigned int character = 0;
};

struct EventDeviceChange
{
};

using Event = std::variant<
	EventUnknown,
	EventQuit,
	EventResize,
	EventFocus,
	EventKey,
	EventMouseMove,
	EventMouseButton,
	EventMouseWheel,
	EventMouseWindow,
	EventCharacter,
	EventDeviceChange>;
}	 // namespace enki