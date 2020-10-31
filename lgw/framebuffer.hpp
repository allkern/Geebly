#pragma once

#include <vector>
#include <cstdint>

#ifdef _WIN32
	#include "SFML\Graphics.hpp"
#endif

#ifdef __linux__
	#include <SFML/Graphics.hpp>
#endif

namespace lgw {
	class framebuffer {
		std::size_t					m_width, m_height, m_effective_width;
		std::vector <uint_least8_t> m_buf;
		sf::Texture					m_tex;
		sf::Sprite					m_spr;

		inline void _init(size_t width, size_t height, sf::Color clear_color = sf::Color::Black) {
			m_buf.reserve(width * height * 4 + 2);
			m_buf.resize(width * height * 4 + 1);

			m_effective_width = width * 4;

			clear();

			m_tex.create(width, height);
			m_tex.update(m_buf.data());
			m_spr.setTexture(m_tex);
		}

	public:
		framebuffer() = default;
		framebuffer(size_t width, size_t height, sf::Color clear_color) { _init(width, height, clear_color); }

		inline void clear(sf::Color clear_color = sf::Color::Black) {
			//log(d, "size = " << std::dec << m_buf.size());
			for (size_t i = 0; i < m_buf.size() - 1;) {
				m_buf[i++] = (uint_least8_t)clear_color.r;
				m_buf[i++] = (uint_least8_t)clear_color.g;
				m_buf[i++] = (uint_least8_t)clear_color.b;
				m_buf[i++] = (uint_least8_t)clear_color.a;
			}
		}

		inline void draw(size_t x, size_t y, sf::Color color) {
			size_t index = (x * 4) + y * m_effective_width;
			
			//log(d, std::dec << "x = " << x << ", y = " << y << ", m_buf[index] = " << (int)m_buf[index-1] << ", index = " << index << ", size = " << m_buf.size())
			m_buf[index++] = (uint_least8_t)color.r;
			m_buf[index++] = (uint_least8_t)color.g;
			m_buf[index++] = (uint_least8_t)color.b;
			m_buf[index  ] = (uint_least8_t)color.a;

		}

		inline void init(size_t width, size_t height, sf::Color clear_color = sf::Color::Black) {
			_init(width, height, clear_color);
		}

		inline void scale(int factor) {
			m_spr.scale(factor, factor);
		}

		inline void move(int x, int y) {
			m_spr.move(sf::Vector2f(x, y));
		}

		inline sf::Sprite* get_drawable() {
			m_tex.update(m_buf.data());
			return &this->m_spr;
		}
	};
}