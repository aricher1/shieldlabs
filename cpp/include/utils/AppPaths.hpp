// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <filesystem>
#include <string>

namespace utils {

    std::filesystem::path find_assets_dir();
    std::filesystem::path asset_path(const std::filesystem::path& relative_path);
    std::string default_user_directory();

} // end namespace utils
