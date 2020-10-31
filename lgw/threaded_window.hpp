#pragma once

#define LGW_VOID void
#define LGW_BOOL bool

#include "basic_window.hpp"
#include "global.hpp"

#include <memory>

namespace lgw {
    class threaded_window : public basic_window {
		sf::Mutex m_window_access_mtx;
		
		// Base objects
		std::shared_ptr <sf::Thread> m_render_thread_sp; // Pointer to a worker thread
		std::shared_ptr <sf::Thread> m_event_thread_sp;

		sf::Event m_event;

		static void s_func_render(threaded_window* ctx) {
			// Get raw pointers to the window and framebuffer
			auto& ctx_window_access_mtx = ctx->m_window_access_mtx;
			
			//LGW_LOCKED_CTX( 
				auto ctx_window = ctx->get_window();
				auto ctx_framebuffer = ctx->m_framebuffer_sp.get();

				ctx_window->setActive(true);
				lgw_log(i, "Activated window");
				#ifdef _WIN32
					ctx_window->setVerticalSyncEnabled(true);
					lgw_log(i, "Vsync enabled");
				#endif
				ctx->setup();
			//)

			while (ctx_window->isOpen()) {
				ctx->draw();
				//LGW_LOCKED_CTX(
					if (ctx->m_using_framebuffer) {
						 ctx_window->draw(*ctx_framebuffer->get_drawable());
					}
					ctx_window->display();
				//)
				LGW_OPTIMIZE_CYCLES
			}
			return;
		}

		static void s_func_event(threaded_window* ctx) {
			lgw_log(i, "Event loop initiated");
			auto& ctx_window_access_mtx = ctx->m_window_access_mtx;

			//LGW_LOCKED_CTX( 
				auto ctx_window = ctx->get_window();
				auto ctx_event = ctx->get_event_ref();
				auto ctx_render_thread = ctx->m_render_thread_sp.get();
			//)

			while (ctx_window->isOpen()) {
				while (ctx_window->pollEvent(ctx_event)) {
					ctx->on_event(ctx_event);
					switch (ctx_event.type) {
						case sf::Event::TextEntered: {
							ctx->on_key(ctx_event.text.unicode);
						} break;
						case sf::Event::Closed:
							ctx->on_close();
						break;
					}
				}
				LGW_OPTIMIZE_CYCLES
			}
			return;
		}

	public:
		threaded_window() = default;
		threaded_window(int width, int height, std::string title = "Untitled", sf::Uint32 st = sf::Style::Default, bool use_framebuffer = true, bool use_threaded_events = false) {
			init(width, height, title, st, use_framebuffer, use_threaded_events);
		}

		inline size_t get_width() { return this->width; }
		inline size_t get_height() { return this->height; }

		inline sf::Event& get_event_ref() { return this->m_event; }
		inline sf::RenderWindow* get_window() { return this->m_window_sp.get(); }

		LGW_VOID init(int width, int height, std::string title = "Untitled", sf::Uint32 st = sf::Style::Default, bool use_framebuffer = true, bool use_threaded_events = false) {
			// Construct a window
			this->m_window_sp = std::make_shared<sf::RenderWindow>(sf::VideoMode(width, height), title);
			lgw_log(i, "Created a window");

			// Initialize framebuffer
			if (use_framebuffer) {
				this->m_using_framebuffer = true;
				this->m_framebuffer_sp = std::make_shared<lgw::framebuffer>(width, height, sf::Color::Magenta);
			}

			// Construct render and event (if use_threaded_events is true) threads
			this->m_render_thread_sp = std::make_shared<sf::Thread>(&s_func_render, this);
			lgw_log(i, "Initialized rendering thread");
			if (use_threaded_events) {
				this->m_event_thread_sp = std::make_shared<sf::Thread>(&s_func_event, this);
				lgw_log(i, "Initialized events thread");
			}
			
			// Deactivate the window
			this->m_window_sp->setActive(false);
			lgw_log(i, "Deactivated window");

			// Set local width and height
			this->width = width;
			this->height = height;
			lgw_log(i, "Initialized buffer");
			
			// Launch the render thread
			this->m_render_thread_sp->launch();
			lgw_log(i, "Launched rendering thread");

			// If use_threaded_events, then launch the events thread, else launch the events function (which blocks main)
			if (use_threaded_events) {
				this->m_event_thread_sp->launch();
			} else {
				this->s_func_event(this);
			}
		}

		LGW_VOID scale(size_t factor) { get_window()->setSize(sf::Vector2u(width*factor, height*factor)); }

		LGW_BOOL is_focused() { /*LGW_LOCKED_LOCAL(*/ return get_window()->hasFocus(); /*)*/ }
		LGW_BOOL is_open() { /*LGW_LOCKED_LOCAL(*/ return get_window()->isOpen(); /*)*/ }
		LGW_VOID update() { get_window()->display(); }
		LGW_VOID close() { close(); m_render_thread_sp->wait(); m_event_thread_sp->wait(); get_window()->~RenderWindow(); }

		LGW_VOID clear (sf::Color color) {
			//LGW_LOCKED_LOCAL(
				if (m_using_framebuffer) {
					m_framebuffer_sp->clear(color);
				}
				get_window()->clear(color);
			//)
		}

		void buffer_draw(size_t x, size_t y, sf::Color color) { m_framebuffer_sp->draw(x, y, color); }
		void buffer_clear(sf::Color color) { m_framebuffer_sp->clear(color); }

	protected:
		virtual inline void on_event(sf::Event&) {};
		virtual inline void setup() {};
		virtual inline void draw() {};
		virtual inline void on_close() {};
		virtual inline void on_key(sf::Uint32) {};
	};
}