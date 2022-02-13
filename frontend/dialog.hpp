#pragma once

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN 1
#endif
#include "Windows.h"
#include "commdlg.h"
#include "WinUser.h"
#endif

#include "geebly/global.hpp"

#include <string>

#define GEEBLY_FD_MAX_FILE_LEN 512

namespace fd {
#ifdef _WIN32
    OPENFILENAMEA ofn;

    const LPOPENFILENAME pofn = &ofn;
    const char* m_filter = "Game Boy\0*.gb\0Game Boy Color\0*.gbc\0\0";
    const char* m_title = "Open";
    const char* m_def_ext = "gb";

    int message_box(const char* text, const char* title, int type, HWND hwnd = NULL) {
        return MessageBox(hwnd, text, title, type);
    }

    std::string open(
        const char* title = "Open",
        const char* def_ext = nullptr,
        const char* filter = nullptr,
        DWORD flags = OFN_PATHMUSTEXIST
    ) {
        m_title = title;
        m_filter = filter;
        m_def_ext = def_ext;

        char file[GEEBLY_FD_MAX_FILE_LEN] = { "\0" };

        std::memset(&ofn, 0, sizeof(OPENFILENAMEA));

        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.lpstrFilter = filter;
        ofn.lpstrFile = &file[0];
        ofn.nMaxFile = GEEBLY_FD_MAX_FILE_LEN;
        ofn.lpstrTitle = title;
        ofn.Flags = flags | OFN_NOCHANGEDIR;
        ofn.lpstrDefExt = def_ext;

        if (!GetOpenFileNameA(&ofn))
            return "";

        return std::string(file);
    }

    std::string save_as(
        const char* title = "Save as",
        const char* def_ext = nullptr,
        const char* filter = nullptr,
        DWORD flags = 0
    ) {
        char file[GEEBLY_FD_MAX_FILE_LEN] = { "\0" };

        std::memset(&ofn, 0, sizeof(OPENFILENAMEA));

        ofn.lStructSize = sizeof(OPENFILENAMEA);
        ofn.lpstrFilter = filter;
        ofn.lpstrFile = &file[0];
        ofn.nMaxFile = GEEBLY_FD_MAX_FILE_LEN;
        ofn.lpstrTitle = title;
        ofn.Flags = flags | OFN_NOCHANGEDIR;
        ofn.lpstrDefExt = def_ext;

        if (!GetSaveFileNameA(&ofn)) {
            return "";
        }

        return std::string(file);
    }
#endif

// Linux file dialogs not implemented yet
#ifdef __linux__
    int message_box(const char* text, const char* title, int type) {
        return 0;
    }

    std::string open(
        const char* title = "Open",
        const char* def_ext = nullptr,
        const char* filter = nullptr,
        DWORD flags = OFN_PATHMUSTEXIST
    ) {
        return "";
    }

    std::string save_as(
        const char* title = "Open",
        const char* def_ext = nullptr,
        const char* filter = nullptr,
        DWORD flags = OFN_PATHMUSTEXIST
    ) {
        return "";
    }
#endif
}