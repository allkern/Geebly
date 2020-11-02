#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <memory>

namespace _log {
    namespace log {
        char buf[512];
        std::string app_name;

        std::ofstream file;

        namespace type {
            const char *none    = "\u001b[30;1m[.]",
                       *debug   = "\u001b[34m[d]",
                       *ok      = "\u001b[32;1m[+]",
                       *info    = "\u001b[34;1m[i]",
                       *warning = "\u001b[35;1m[w]",
                       *error   = "\u001b[31;1m[e]";
        }

        template <class... Args> void log(const char* t, std::string fmt, Args... args) {
            sprintf(buf, fmt.c_str(), args...);
            std::cout << t << "\u001b[0m " + app_name + ": " << buf << std::endl;

            if (file.is_open()) {
                std::string tstr(t), l = tstr.substr(tstr.find_last_of('['), 3) + " ";
                file << l << buf << std::endl;
            }
        }

        void init(std::string app, const std::string& fn = "") {
            app_name = app;
            if (fn.size()) {
                file.open(fn);
                if (!file.is_open()) {
                    log(type::warning, "Couldn't open log file \"%s\"", fn.c_str());
                }
            }
        }
    }
}

#define _log(t, ...) _log::log::log(_log::log::type::t, __VA_ARGS__)