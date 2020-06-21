#include "RendererSFML.hpp"

//STD
#include <algorithm>
#include <execution>

//SELF
#include "Enki/Window/WindowSFML.hpp"

namespace enki
{

RendererSFML::RendererSFML(Window* window)
	: Renderer(type, window)
{
}

void RendererSFML::draw(const Sprite* s)
{
	Helper h;
	h.order_info = &s->order_info;
	auto sprite = std::make_unique<sf::Sprite>();
	sf::Color colour;
	colour.r = s->colour.r * 255;
	colour.g = s->colour.g * 255;
	colour.b = s->colour.b * 255;
	colour.a = s->colour.a * 255;
	sprite->setColor(std::move(colour));
	//sprite->setTexture() //todo
	sprite->setOrigin(0.5f, 0.5f); //todo: user might want something different
	sprite->setPosition(s->transform.pos_x, s->transform.pos_y);
	sprite->setScale(s->transform.scale_x, s->transform.scale_y);
	sprite->setRotation(s->transform.angle);
	h.drawable = std::move(sprite);
	helpers.emplace_back(std::move(h));
}

void RendererSFML::draw(const Text* t)
{
	Helper h;
	h.order_info = &t->order_info;
	auto text = std::make_unique<sf::Text>();
	sf::Color colour;
	colour.r = t->colour.r * 255;
	colour.g = t->colour.g * 255;
	colour.b = t->colour.b * 255;
	colour.a = t->colour.a * 255;
	sf::Color outline_colour;
	outline_colour.r = t->colour_outline.r * 255;
	outline_colour.g = t->colour_outline.g * 255;
	outline_colour.b = t->colour_outline.b * 255;
	outline_colour.a = t->colour_outline.a * 255;
	text->setFillColor(colour);
	text->setOutlineColor(outline_colour);
	text->setOrigin(0.5f, 0.5f);	//todo: user might want something different
	text->setPosition(t->transform.pos_x, t->transform.pos_y);
	text->setScale(t->transform.scale_x, t->transform.scale_y);
	text->setRotation(t->transform.angle);
	text->setString(t->text);
	//text->setStyle(); //todo: 
	//text->setOutlineThickness(); //todo: 
	//text->setLineSpacing(); //todo: 
	//text->setLetterSpacing(); //todo: 
	//text->setFont() //todo: 
	//text->setCharacterSize() //todo: 
	h.drawable = std::move(text);
	helpers.emplace_back(std::move(h));
}

void RendererSFML::end()
{
	std::sort(std::execution::par_unseq, helpers.begin(), helpers.end(), 
		[](const Helper& left, const Helper& right) {
		if (left.order_info->layer == right.order_info->layer)
			return left.order_info->order < right.order_info->order;
		
		return left.order_info->layer < right.order_info->layer;
	});

	for (const auto& h : helpers)
		window->as<WindowSFML>()->getRawWindow()->draw(*h.drawable);

	helpers.clear();
}

}	// namespace enki