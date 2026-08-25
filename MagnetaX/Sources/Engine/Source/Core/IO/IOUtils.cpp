// Copyright (c) 2026 PulsaX Szymon Krygier
// SPDX-License-Identifier: MPL-2.0
#include <MX/Core/IO/IOUtils.h>

#if MX_PLATFORM_WINDOWS
    #include <Windows.h>
#elif MX_PLATFORM_LINUX
    #include <unistd.h>
#elif MX_PLATFORM_APPLE
    #include <mach-o/dyld.h>
#endif

#include <fstream>
#include <string>
#include <vector>

#if MX_PLATFORM_WINDOWS
std::filesystem::path IOUtils::GetExecutablePath()
{
    wchar_t pathBuffer[MAX_PATH]{};
    const DWORD pathLength = GetModuleFileNameW(nullptr, pathBuffer, MAX_PATH);

    if (pathLength == 0) return {};

    return std::filesystem::path(pathBuffer, pathBuffer + pathLength);
}
#elif MX_PLATFORM_LINUX
std::filesystem::path IOUtils::GetExecutablePath()
{
    char pathBuffer[4096]{};
    const ssize_t pathLength = readlink("/proc/self/exe", pathBuffer, sizeof(pathBuffer));

    if (pathLength <= 0) return {};

    return std::filesystem::path(std::string(pathBuffer, static_cast<usize>(pathLength)));
}
#elif MX_PLATFORM_APPLE
std::filesystem::path IOUtils::GetExecutablePath()
{
    uint32 pathSize = 0;
    _NSGetExecutablePath(nullptr, &pathSize);

    std::vector<char> pathBuffer(pathSize);

    if (_NSGetExecutablePath(pathBuffer.data(), &pathSize) != 0) return {};

    return std::filesystem::weakly_canonical(pathBuffer.data());
}
#else
std::filesystem::path IOUtils::GetExecutablePath()
{
    return {};
}
#endif

std::filesystem::path IOUtils::GetExecutableDirectory()
{
    return GetExecutablePath().parent_path();
}

bool IOUtils::ReadBinaryFile(const std::filesystem::path& path, std::vector<uint8>& data)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return false;

    const std::streamsize fileSize = file.tellg();
    if (fileSize <= 0) return false;

    file.seekg(0, std::ios::beg);
    data.resize(static_cast<usize>(fileSize));

    if (!file.read(reinterpret_cast<char*>(data.data()), fileSize))
    {
        data.clear();
        return false;
    }

    return true;
}
