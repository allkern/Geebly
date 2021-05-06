#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

#include "log.hpp"

namespace cli {
    std::vector <std::string> cli;
    std::unordered_map <std::string, std::string> settings;

    namespace detail {
        static inline std::string ltrim(std::string s) {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
                return !std::isspace(ch);
            }));
            return s; 
        }

        static inline std::string rtrim(std::string s) {
            s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
                return !std::isspace(ch);
            }).base(), s.end());
            return s;
        }

        static inline std::string trim(std::string s) {
            return ltrim(rtrim(s));
        }

        long find_switch(std::string sw, std::string alt) {
            long i = 0;
            for (std::string& s : cli) {
                if ((s == sw) || (s == alt)) {
                    cli.erase(cli.begin()+i);
                    return i;
                }
                i++;
            }
            return -1;
        }
    }

    bool is_defined(std::string setting) {
        return settings.find(setting) != settings.end();
    }

    std::string setting(std::string key, std::string def) {
        return is_defined(key) ? settings[key] : def;
    }

    bool setting(std::string key) {
        return is_defined(key);
    }

    void init(size_t argc, char* argv[]) {
        if (argc) {
            cli.reserve(argc-1);

            for (int i = 1; i < argc; i++) {
                cli.push_back(std::string(argv[i]));
            }
        }
    }

#define DEFINE_SETTING(clsw, alt, id) \
    i = detail::find_switch(clsw, alt); if (i > -1) { settings.insert({id, cli.at(i)}); cli.erase(cli.begin()+i); }

#define DEFINE_SWITCH(clsw, alt, id) \
    i = detail::find_switch(clsw, alt); if (i > -1) { settings.insert({id, "true"}); }

    void parse() {
        if (cli.size()) {
            int i = 0;
            
            DEFINE_SETTING("--boot"         , "-b", "boot");
            DEFINE_SETTING("--scale"        , "-s", "scale");
            DEFINE_SETTING("--master-volume", "-v", "master-volume");

            DEFINE_SWITCH("--debug"                   , "-d"                 , "debug");
            DEFINE_SWITCH("--enable-joyp-irq-delay"   , "-Ejoyp-irq-delay"   , "joyp-irq-delay");
            DEFINE_SWITCH("--disable-joyp-irq-delay"  , "-Eno-joyp-irq-delay", "no-joyp-irq-delay");
            DEFINE_SWITCH("--skip-bootrom"            , "-Bskip"             , "bootrom-skip");
            DEFINE_SWITCH("--cgb-mode", "-Ecgb"       , "cgb-mode");
            DEFINE_SWITCH("--patch-bios-checks"       , "-Bno-checks"        , "no-bios-checks");
            DEFINE_SWITCH("--no-patch-bios-checks"    , "-Bchecks"           , "bios-checks");
            DEFINE_SWITCH("--vram-access-emulation"   , "-Gvram-access"      , "vram-access-emulation");
            DEFINE_SWITCH("--no-vram-access-emulation", "-Gno-vram-access"   , "no-vram-access-emulation");
            DEFINE_SWITCH("--mono"                    , "-m"                 , "mono");
            DEFINE_SWITCH("--sound-disabled"          , "-Sdisable"          , "sound-disabled");

            if (cli.size()) {
                if (cli.at(0).size()) {
                    settings.insert({"cartridge", cli.at(0)});
                }
            }
        }
    }
#undef DEFINE_SETTING
#undef DEFINE_SWITCH
}