#include "platform.hpp"

#include <vector>

Platform::Platform(const char* title, const int window_width, const int window_height, const int texture_width,
                   const int texture_height)
	: m_window(sf::VideoMode({static_cast<unsigned int>(window_width), static_cast<unsigned int>(window_height)}),
	           title),
	  m_sprite(m_texture)
{
	(void)m_texture.resize(sf::Vector2u(static_cast<unsigned int>(texture_width),
	                                    static_cast<unsigned int>(texture_height)));
	m_sprite.setTexture(m_texture, true);

	const float scale_x = static_cast<float>(window_width) / static_cast<float>(texture_width);
	const float scale_y = static_cast<float>(window_height) / static_cast<float>(texture_height);
	m_sprite.setScale(sf::Vector2f(scale_x, scale_y));
}

void Platform::update(const uint8_t* buffer)
{
	std::vector<std::uint8_t> rgba_pixels(Chip8::FRAME_BUF_W * Chip8::FRAME_BUF_H * 4);

	for (size_t i = 0; i < Chip8::FRAME_BUF_W * Chip8::FRAME_BUF_H; ++i)
	{
		const uint8_t color = buffer[i] ? 255 : 0;
		rgba_pixels[i * 4 + 0] = color;
		rgba_pixels[i * 4 + 1] = color;
		rgba_pixels[i * 4 + 2] = color;
		rgba_pixels[i * 4 + 3] = 255;
	}

	m_texture.update(rgba_pixels.data());
	m_window.clear();
	m_window.draw(m_sprite);
	m_window.display();
}

bool Platform::process_input(Chip8& chip8)
{
	bool quit = false;

	while (std::optional<sf::Event> event = m_window.pollEvent())
	{
		if (event->getIf<sf::Event::Closed>())
		{
			quit = true;
		}
		else if (const auto* key_pressed = event->getIf<sf::Event::KeyPressed>())
		{
			switch (key_pressed->code)
			{
			case sf::Keyboard::Key::Escape:
				quit = true;
				break;
			case sf::Keyboard::Key::X:
				chip8.set_key(0x0, true);
				break;
			case sf::Keyboard::Key::Num1:
				chip8.set_key(0x1, true);
				break;
			case sf::Keyboard::Key::Num2:
				chip8.set_key(0x2, true);
				break;
			case sf::Keyboard::Key::Num3:
				chip8.set_key(0x3, true);
				break;
			case sf::Keyboard::Key::Q:
				chip8.set_key(0x4, true);
				break;
			case sf::Keyboard::Key::W:
				chip8.set_key(0x5, true);
				break;
			case sf::Keyboard::Key::E:
				chip8.set_key(0x6, true);
				break;
			case sf::Keyboard::Key::A:
				chip8.set_key(0x7, true);
				break;
			case sf::Keyboard::Key::S:
				chip8.set_key(0x8, true);
				break;
			case sf::Keyboard::Key::D:
				chip8.set_key(0x9, true);
				break;
			case sf::Keyboard::Key::Z:
				chip8.set_key(0xA, true);
				break;
			case sf::Keyboard::Key::C:
				chip8.set_key(0xB, true);
				break;
			case sf::Keyboard::Key::Num4:
				chip8.set_key(0xC, true);
				break;
			case sf::Keyboard::Key::R:
				chip8.set_key(0xD, true);
				break;
			case sf::Keyboard::Key::F:
				chip8.set_key(0xE, true);
				break;
			case sf::Keyboard::Key::V:
				chip8.set_key(0xF, true);
				break;
			default:
				break;
			}
		}
		else if (const auto* key_released = event->getIf<sf::Event::KeyReleased>())
		{
			switch (key_released->code)
			{
			case sf::Keyboard::Key::X:
				chip8.set_key(0x0, false);
				break;
			case sf::Keyboard::Key::Num1:
				chip8.set_key(0x1, false);
				break;
			case sf::Keyboard::Key::Num2:
				chip8.set_key(0x2, false);
				break;
			case sf::Keyboard::Key::Num3:
				chip8.set_key(0x3, false);
				break;
			case sf::Keyboard::Key::Q:
				chip8.set_key(0x4, false);
				break;
			case sf::Keyboard::Key::W:
				chip8.set_key(0x5, false);
				break;
			case sf::Keyboard::Key::E:
				chip8.set_key(0x6, false);
				break;
			case sf::Keyboard::Key::A:
				chip8.set_key(0x7, false);
				break;
			case sf::Keyboard::Key::S:
				chip8.set_key(0x8, false);
				break;
			case sf::Keyboard::Key::D:
				chip8.set_key(0x9, false);
				break;
			case sf::Keyboard::Key::Z:
				chip8.set_key(0xA, false);
				break;
			case sf::Keyboard::Key::C:
				chip8.set_key(0xB, false);
				break;
			case sf::Keyboard::Key::Num4:
				chip8.set_key(0xC, false);
				break;
			case sf::Keyboard::Key::R:
				chip8.set_key(0xD, false);
				break;
			case sf::Keyboard::Key::F:
				chip8.set_key(0xE, false);
				break;
			case sf::Keyboard::Key::V:
				chip8.set_key(0xF, false);
				break;
			default:
				break;
			}
		}
	}

	return quit;
}
