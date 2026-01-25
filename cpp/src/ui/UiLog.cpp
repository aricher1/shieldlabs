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