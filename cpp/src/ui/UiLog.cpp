// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include "ui/UiLog.hpp"


namespace ui {

    UiLog ui_log;

    void UiLog::push(const std::string& s) {
        lines.push_back(s);;
        if (lines.size() > max_lines) {
            lines.erase(lines.begin());
        }
    }

} // end namespace ui