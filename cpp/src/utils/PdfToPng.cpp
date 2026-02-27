// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <filesystem>
#include <memory>
#include <poppler/cpp/poppler-document.h>
#include <poppler/cpp/poppler-page.h>
#include <poppler/cpp/poppler-page-renderer.h>

#include "utils/PdfToPng.hpp"


namespace utils {

    bool pdf_to_png(const std::string& pdf_path, const std::string& png_path, int dpi) {
        std::unique_ptr<poppler::document> doc(poppler::document::load_from_file(pdf_path));

        if (!doc) {
            return false;
        }
        if (doc->is_locked()) {
            return false;
        }

        if (doc->pages() != 1) {
            return false;
        }
        std::unique_ptr<poppler::page> page(doc->create_page(0));

        if (!page) {
            return false;
        }
        
        poppler::page_renderer renderer;
        renderer.set_render_hint(poppler::page_renderer::antialiasing, true);
        renderer.set_render_hint(poppler::page_renderer::text_antialiasing, true);

        poppler::image img = renderer.render_page(page.get(), dpi, dpi);

        if (!img.is_valid() || img.width() == 0 || img.height() == 0) {
            return false;
        }

        std::filesystem::create_directories(std::filesystem::path(png_path).parent_path());

        return img.save(png_path, "png");
    }

} // end namespace utils