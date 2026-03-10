// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include "utils/AppPaths.hpp"

#include <array>
#include <cstdlib>

namespace utils {

    namespace fs = std::filesystem;

    namespace {

        fs::path first_existing_path(const std::array<fs::path, 5>& candidates) {
            for (const auto& candidate : candidates) {
                if (!candidate.empty() && fs::exists(candidate)) {
                    return candidate;
                }
            }

            return {};
        }

    } // namespace

    fs::path find_assets_dir() {
        const fs::path cwd = fs::current_path();
        const fs::path discovered = first_existing_path({
            cwd / "assets",
            cwd / "../assets",
            cwd / "../../assets",
            cwd / "cpp/assets",
            cwd / "../cpp/assets"
        });

        return discovered.empty() ? (cwd / "assets") : discovered;
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
