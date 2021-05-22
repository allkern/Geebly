#pragma once

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include "Windows.h"
#include "commdlg.h"
#endif

#include "geebly/global.hpp"

#include <string>

#define GEEBLY_FD_MAX_FILE_LEN 512

namespace fd {
#ifdef _WIN32
    OPENFILENAMEA ofn;

    const LPOPENFILENAME pofn = &ofn;
    const char* filter = "Game Boy\0*.gb\0Game Boy Color\0*.gbc\0\0";
    const char* title = "Open";
    const char* def_ext = "gb";

    std::string open() {
        char file[GEEBLY_FD_MAX_FILE_LEN] = { "\0" };

        std::memset(&ofn, 0, sizeof(OPENFILENAMEA));

        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.lpstrFilter = filter;
        ofn.lpstrFile = &file[0];
        ofn.nMaxFile = GEEBLY_FD_MAX_FILE_LEN;
        ofn.lpstrTitle = title;
        ofn.Flags = OFN_PATHMUSTEXIST;
        ofn.lpstrDefExt = def_ext;

        if (!GetOpenFileNameA(&ofn)) {
            return "";
        }

        return std::string(file);
    }
#endif

#ifdef __linux__
    std::string open() {
        return "";
    }
#endif
}