// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <filesystem>
#include <memory>

#if __has_include(<poppler/cpp/poppler-document.h>)
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>
#elif __has_include(<poppler-document.h>)
#include <poppler-document.h>
#include <poppler-page.h>
#include <poppler-page-renderer.h>
#else
#error "Poppler C++ headers not found"
#endif

#include "utils/PdfToPng.hpp"


namespace utils {

    bool pdf_to_png(const std::string& pdf_path, const std::string& png_path, int dpi, std::string* error_message) {
        const auto set_error = [&](const std::string& message) {
            if (error_message) {
                *error_message = message;
            }
        };

        if (!std::filesystem::exists(pdf_path)) {
            set_error("Selected PDF file does not exist.");
            return false;
        }

        std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdf_path));

        if (!doc) {
            set_error("Poppler could not open the selected PDF file.");
            return false;
        }
        if (doc->is_locked()) {
            set_error("The selected PDF is encrypted or locked.");
            return false;
        }

        if (doc->pages() != 1) {
            set_error("The selected PDF must contain exactly one page.");
            return false;
        }
        std::unique_ptr<poppler::page> page(doc->create_page(0));

        if (!page) {
            set_error("Poppler could not read the first page.");
            return false;
        }
        
        poppler::page_renderer renderer;
        renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
        renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);

        poppler::image img = renderer.render_page(page.get(), dpi, dpi);

        if (!img.is_valid() || img.width() == 0 || img.height() == 0) {
            set_error("The PDF rendered as an empty or invalid image.");
            return false;
        }

        std::filesystem::create_directories(std::filesystem::path(png_path).parent_path());

        if (!img.save(png_path, "png")) {
            set_error("The rendered image could not be written to disk.");
            return false;
        }

        if (error_message) {
            error_message->clear();
        }

        return true;
    }

} // end namespace utils
