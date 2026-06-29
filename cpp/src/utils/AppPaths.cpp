// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include "utils/AppPaths.hpp"

#include <array>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

namespace utils {

    namespace fs = std::filesystem;

    namespace {

        fs::path first_existing_path(const std::array<fs::path, 8>& candidates) {
            for (const auto& candidate : candidates) {
                if (!candidate.empty() && fs::exists(candidate)) {
                    return candidate;
                }
            }

            return {};
        }

        fs::path executable_dir() {
#ifdef _WIN32
            std::wstring buffer(MAX_PATH, L'\0');
            const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
            if (length == 0) {
                return {};
            }

            buffer.resize(length);
            return fs::path(buffer).parent_path();
#else
            return {};
#endif
        }

    } // namespace

    fs::path find_assets_dir() {
        const fs::path cwd = fs::current_path();
        const fs::path exe_dir = executable_dir();
        const fs::path discovered = first_existing_path({
            exe_dir / "assets",
            exe_dir / "../assets",
            exe_dir / "../../assets",
            exe_dir / "../../../assets",
            cwd / "assets",
            cwd / "../assets",
            cwd / "../../assets",
            cwd / "cpp/assets"
        });

        return discovered.empty() ? (exe_dir.empty() ? (cwd / "assets") : (exe_dir / "assets")) : discovered;
    }

    fs::path asset_path(const fs::path& relative_path) {
        return find_assets_dir() / relative_path;
    }

    std::string default_user_directory() {
#ifdef _WIN32
        if (const char* profile = std::getenv("USERPROFILE")) {
            return profile;
        }

        const char* drive = std::getenv("HOMEDRIVE");
        const char* path = std::getenv("HOMEPATH");
        if (drive && path) {
            return fs::path(std::string(drive) + path).string();
        }
#else
        if (const char* home = std::getenv("HOME")) {
            return home;
        }
#endif

        return fs::current_path().string();
    }

} // end namespace utils
