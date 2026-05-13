#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <cstdint>
#include <optional>
#include <vector>

#include "chip8.hpp"

class Platform
{
public:
	Platform(char const* title, const int windowWidth, const int windowHeight, const int textureWidth,
	         const int textureHeight)
		: window(sf::VideoMode({static_cast<unsigned int>(windowWidth), static_cast<unsigned int>(windowHeight)}),
		         title),
		  sprite(texture)
	{
		(void)texture.resize(sf::Vector2u(static_cast<unsigned int>(textureWidth),
		                                  static_cast<unsigned int>(textureHeight)));
		sprite.setTexture(texture, true);

		const float scaleX = static_cast<float>(windowWidth) / static_cast<float>(textureWidth);
		const float scaleY = static_cast<float>(windowHeight) / static_cast<float>(textureHeight);
		sprite.setScale(sf::Vector2f(scaleX, scaleY));
	}

	~Platform() = default;

	void update(void const* buffer, [[maybe_unused]] int pitch)
	{
		std::vector<std::uint8_t> rgba_pixels(Chip8::FRAME_BUF_W * Chip8::FRAME_BUF_H * 4);
		const auto* raw_buffer = static_cast<const std::uint8_t*>(buffer);

		for (size_t i = 0; i < Chip8::FRAME_BUF_W * Chip8::FRAME_BUF_H; ++i)
		{
			const std::uint8_t color = raw_buffer[i] ? 255 : 0;

			rgba_pixels[i * 4 + 0] = color; // R
			rgba_pixels[i * 4 + 1] = color; // G
			rgba_pixels[i * 4 + 2] = color; // B
			rgba_pixels[i * 4 + 3] = 255; // A (Fully opaque)
		}

		texture.update(rgba_pixels.data());
		window.clear();
		window.draw(sprite);
		window.display();
	}

	bool process_input(uint8_t* keys)
	{
		bool quit = false;

		while (std::optional<sf::Event> event = window.pollEvent())
		{
			if (event->getIf<sf::Event::Closed>())
			{
				quit = true;
			}
			else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
			{
				switch (keyPressed->code)
				{
				case sf::Keyboard::Key::Escape:
					quit = true;
					break;
				case sf::Keyboard::Key::X:
					keys[0] = 1;
					break;
				case sf::Keyboard::Key::Num1:
					keys[1] = 1;
					break;
				case sf::Keyboard::Key::Num2:
					keys[2] = 1;
					break;
				case sf::Keyboard::Key::Num3:
					keys[3] = 1;
					break;
				case sf::Keyboard::Key::Q:
					keys[4] = 1;
					break;
				case sf::Keyboard::Key::W:
					keys[5] = 1;
					break;
				case sf::Keyboard::Key::E:
					keys[6] = 1;
					break;
				case sf::Keyboard::Key::A:
					keys[7] = 1;
					break;
				case sf::Keyboard::Key::S:
					keys[8] = 1;
					break;
				case sf::Keyboard::Key::D:
					keys[9] = 1;
					break;
				case sf::Keyboard::Key::Z:
					keys[0xA] = 1;
					break;
				case sf::Keyboard::Key::C:
					keys[0xB] = 1;
					break;
				case sf::Keyboard::Key::Num4:
					keys[0xC] = 1;
					break;
				case sf::Keyboard::Key::R:
					keys[0xD] = 1;
					break;
				case sf::Keyboard::Key::F:
					keys[0xE] = 1;
					break;
				case sf::Keyboard::Key::V:
					keys[0xF] = 1;
					break;
				default:
					break;
				}
			}
			else if (const auto* keyReleased = event->getIf<sf::Event::KeyReleased>())
			{
				switch (keyReleased->code)
				{
				case sf::Keyboard::Key::X:
					keys[0] = 0;
					break;
				case sf::Keyboard::Key::Num1:
					keys[1] = 0;
					break;
				case sf::Keyboard::Key::Num2:
					keys[2] = 0;
					break;
				case sf::Keyboard::Key::Num3:
					keys[3] = 0;
					break;
				case sf::Keyboard::Key::Q:
					keys[4] = 0;
					break;
				case sf::Keyboard::Key::W:
					keys[5] = 0;
					break;
				case sf::Keyboard::Key::E:
					keys[6] = 0;
					break;
				case sf::Keyboard::Key::A:
					keys[7] = 0;
					break;
				case sf::Keyboard::Key::S:
					keys[8] = 0;
					break;
				case sf::Keyboard::Key::D:
					keys[9] = 0;
					break;
				case sf::Keyboard::Key::Z:
					keys[0xA] = 0;
					break;
				case sf::Keyboard::Key::C:
					keys[0xB] = 0;
					break;
				case sf::Keyboard::Key::Num4:
					keys[0xC] = 0;
					break;
				case sf::Keyboard::Key::R:
					keys[0xD] = 0;
					break;
				case sf::Keyboard::Key::F:
					keys[0xE] = 0;
					break;
				case sf::Keyboard::Key::V:
					keys[0xF] = 0;
					break;
				default:
					break;
				}
			}
		}

		return quit;
	}

private:
	sf::RenderWindow window;
	sf::Texture texture;
	sf::Sprite sprite;
};
