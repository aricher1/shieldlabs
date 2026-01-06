#include "ui/UiLog.hpp"

UiLog ui_log;

void UiLog::push(const std::string& s) {
    lines.push_back(s);;
    if (lines.size() > max_lines) {
        lines.erase(lines.begin());
    }
}