#pragma once

#include <string>



namespace utils {

    bool pdf_to_png(const std::string& pdf_path, const std::string& png_path, int dpi = 150);

} // end namespace utils