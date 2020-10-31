#include <cstdint>
#include <memory>

#ifdef _WIN32
	#include "SFML\Graphics.hpp"
#endif

#ifdef __linux__
	#include <SFML/Graphics.hpp>
#endif

#include "framebuffer.hpp"

namespace lgw {
    class basic_window {
	public:
		std::size_t width, height;

	protected:
		std::shared_ptr <sf::RenderWindow> m_window_sp;
		std::shared_ptr <lgw::framebuffer> m_framebuffer_sp;
		bool m_initialized = false,
			 m_using_framebuffer = false;
	};
}