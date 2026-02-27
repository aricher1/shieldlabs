// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <cstddef>
#include <deque>
#include <string>



namespace ui {

    struct UiLog {
        std::deque<std::string> lines;
        std::size_t max_lines = 1000;

        void clear() {
            lines.clear();
        }

        void push(const std::string& s);

        void separator() {
            push("-----------");
        }
    };

    // Global UI log
    extern UiLog ui_log;

} // end namespace ui