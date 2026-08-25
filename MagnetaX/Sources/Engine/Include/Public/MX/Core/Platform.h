// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#pragma once

#undef MX_PLATFORM_WINDOWS
#undef MX_PLATFORM_LINUX
#undef MX_PLATFORM_APPLE
#undef MX_PLATFORM_UNKNOWN

// Those ifs won't exclude mobile platforms I believe
// Linux will probably include Android and Apple will include iOS also
// TODO: check it later and exclude them somehow
// https://blog.kowalczyk.info/article/j/guide-to-predefined-macros-in-c-compilers-gcc-clang-msvc-etc..html
#if defined(_WIN32) || defined(_WIN64)
    #define MX_PLATFORM_WINDOWS 1
#elif defined(__linux__)
    #define MX_PLATFORM_LINUX 1
#elif defined(__APPLE__)
    #define MX_PLATFORM_APPLE 1
#else
    #define MX_PLATFORM_UNKNOWN 1
#endif

#if !defined(MX_PLATFORM_WINDOWS)
    #define MX_PLATFORM_WINDOWS 0
#endif

#if !defined(MX_PLATFORM_LINUX)
    #define MX_PLATFORM_LINUX 0
#endif

#if !defined(MX_PLATFORM_APPLE)
    #define MX_PLATFORM_APPLE 0
#endif

#if !defined(MX_PLATFORM_UNKNOWN)
    #define MX_PLATFORM_UNKNOWN 0
#endif
