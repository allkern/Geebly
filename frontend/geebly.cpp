#ifdef __linux__
#define LOG_TARGET_LINUX 1
#else
#define LOG_TARGET_LINUX 0
#endif

#ifdef _WIN32
#define LOG_TARGET_POWERSHELL 1
#else
#define LOG_TARGET_POWERSHELL 0
#endif

#define GEEBLY_FRAMEBUFFER_FORMAT 2

#include "geebly/gameboy.hpp"
#include "geebly/log.hpp"

#include "frontend.hpp"

using namespace gameboy;
using namespace frontend;

int main(int argc, char *argv[]) {
    _log::init("geebly");

    cli::init(argc, argv);
    cli::parse();

    frontend::load_settings();

    if (settings::debugger_enabled) debug::start();

    frontend::init();
    gameboy::init();

    while (screen::is_open()) gameboy::update();

    frontend::close();

    cart::create_sav_file();

    return 0;
}
