#pragma once

#ifdef _WIN32
    #include <windows.h>

    // Helper for getting the system DPI scale on Windows
    inline float getSystemScale() {
        HDC dc = GetDC(nullptr);
        int dpi = GetDeviceCaps(dc, LOGPIXELSX);
        ReleaseDC(nullptr, dc);

        return dpi / 96.0f;
    }

#elif defined(__APPLE__)

    inline float getSystemScale() {
        return 2.0f;
    }

#endif