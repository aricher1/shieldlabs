// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <string>



namespace utils {

    bool pdf_to_png(const std::string& pdf_path, const std::string& png_path, int dpi = 150, std::string* error_message = nullptr);

} // end namespace utils
