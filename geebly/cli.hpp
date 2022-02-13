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

    bool no_args = false;

    bool no_arguments() {
        return no_args;
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
            DEFINE_SWITCH("--enable-joyp-irq-delay"   , "-joyp-irq-delay"    , "joyp-irq-delay");
            DEFINE_SWITCH("--disable-joyp-irq-delay"  , "-no-joyp-irq-delay" , "no-joyp-irq-delay");
            DEFINE_SWITCH("--skip-bootrom"            , "-no-boot"           , "bootrom-skip");
            DEFINE_SWITCH("--cgb-mode"                , "-cgb"               , "cgb-mode");
            DEFINE_SWITCH("--patch-bios-checks"       , "-no-checks"         , "no-bios-checks");
            DEFINE_SWITCH("--no-patch-bios-checks"    , "-checks"            , "bios-checks");
            DEFINE_SWITCH("--vram-access-emulation"   , "-vram-access"       , "vram-access-emulation");
            DEFINE_SWITCH("--no-vram-access-emulation", "-no-vram-access"    , "no-vram-access-emulation");
            DEFINE_SWITCH("--mono"                    , "-mono"              , "mono");
            DEFINE_SWITCH("--sound-disabled"          , "-m"                 , "sound-disabled");
            DEFINE_SWITCH("--no-logs"                 , "-n"                 , "no-logs");
            DEFINE_SWITCH("--ntsc-codec"              , "-ntsc"              , "ntsc-codec");
            DEFINE_SWITCH("--sgb-mode"                , "-sgb"               , "sgb-mode");
            DEFINE_SWITCH("--gui"                     , "-g"                 , "gui");
            DEFINE_SWITCH("--blend-frames"            , "-i"                 , "blend-frames");

            if (cli.size()) {
                if (cli.at(0).size()) {
                    settings.insert({"cartridge", cli.at(0)});
                }
            }
        } else {
            no_args = true;
        }
    }
#undef DEFINE_SETTING
#undef DEFINE_SWITCH
}