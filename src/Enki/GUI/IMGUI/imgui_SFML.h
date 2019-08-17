#pragma once

//SELF
#include "imgui.h"

namespace sf
{
    class Event;
    class RenderTarget;
    class RenderWindow;
    class Sprite;
    class Texture;
    class Window;
}

namespace ImGui
{
    namespace SFML
    {
        void Init(sf::RenderWindow& window, bool loadDefaultFont = true);
        void Init(sf::Window& window, sf::RenderTarget& target, bool loadDefaultFont = true);
        void Init(sf::Window& window, const sf::Vector2f& displaySize, bool loadDefaultFont = true);

		//returns true if the input should be consumed (and not passed onwards to the rest of the game)
        bool ProcessEvent(const sf::Event& event);

        void Update(sf::RenderWindow& window, float dt);
        void Update(sf::Window& window, sf::RenderTarget& target, float dt);
        void Update(const sf::Vector2i& mousePos, const sf::Vector2f& displaySize, float dt);

        void Render(sf::RenderTarget& target);
        void Render();

        void Shutdown();

        void UpdateFontTexture();
        sf::Texture& GetFontTexture();

        // joystick functions
        void SetActiveJoystickId(unsigned int joystickId);
        void SetJoytickDPadThreshold(float threshold);
        void SetJoytickLStickThreshold(float threshold);

        void SetJoystickMapping(int action, unsigned int joystickButton);
        void SetDPadXAxis(sf::Joystick::Axis dPadXAxis, bool inverted = false);
        void SetDPadYAxis(sf::Joystick::Axis dPadYAxis, bool inverted = false);
        void SetLStickXAxis(sf::Joystick::Axis lStickXAxis, bool inverted = false);
        void SetLStickYAxis(sf::Joystick::Axis lStickYAxis, bool inverted = false);
    }

    // custom ImGui widgets for SFML stuff

    // Image overloads
    void Image(const sf::Texture& texture,
        const sf::Color& tintColor = sf::Color::White,
        const sf::Color& borderColor = sf::Color::Transparent);
    void Image(const sf::Texture& texture, const sf::Vector2f& size,
        const sf::Color& tintColor = sf::Color::White,
        const sf::Color& borderColor = sf::Color::Transparent);
    void Image(const sf::Texture& texture, const sf::FloatRect& textureRect,
        const sf::Color& tintColor = sf::Color::White,
        const sf::Color& borderColor = sf::Color::Transparent);
    void Image(const sf::Texture& texture, const sf::Vector2f& size, const sf::FloatRect& textureRect,
        const sf::Color& tintColor = sf::Color::White,
        const sf::Color& borderColor = sf::Color::Transparent);

    void Image(const sf::Sprite& sprite,
        const sf::Color& tintColor = sf::Color::White,
        const sf::Color& borderColor = sf::Color::Transparent);
    void Image(const sf::Sprite& sprite, const sf::Vector2f& size,
        const sf::Color& tintColor = sf::Color::White,
        const sf::Color& borderColor = sf::Color::Transparent);

    // ImageButton overloads
    bool ImageButton(const sf::Texture& texture, const int framePadding = -1,
        const sf::Color& bgColor = sf::Color::Transparent,
        const sf::Color& tintColor = sf::Color::White);
    bool ImageButton(const sf::Texture& texture, const sf::Vector2f& size, const int framePadding = -1,
        const sf::Color& bgColor = sf::Color::Transparent, const sf::Color& tintColor = sf::Color::White);

    bool ImageButton(const sf::Sprite& sprite, const int framePadding = -1,
        const sf::Color& bgColor = sf::Color::Transparent,
        const sf::Color& tintColor = sf::Color::White);
    bool ImageButton(const sf::Sprite& sprite, const sf::Vector2f& size, const int framePadding = -1,
        const sf::Color& bgColor = sf::Color::Transparent,
        const sf::Color& tintColor = sf::Color::White);

    // Draw_list overloads. All positions are in relative coordinates (relative to top-left of the current window)
    void DrawLine(const sf::Vector2f& a, const sf::Vector2f& b, const sf::Color& col, float thickness = 1.0f);
    void DrawRect(const sf::FloatRect& rect, const sf::Color& color, float rounding = 0.0f, int rounding_corners = 0x0F, float thickness = 1.0f);
    void DrawRectFilled(const sf::FloatRect& rect, const sf::Color& color, float rounding = 0.0f, int rounding_corners = 0x0F);
}