#pragma once
#include <deque>
#include <string>

struct UiLog {
    std::deque<std::string> lines;
    std::size_t max_lines = 500;

    void clear() {
        lines.clear();
    }

    void push(const std::string& s);

    void separator() {
        push("-------------------------------");
    }
};

extern UiLog ui_log;