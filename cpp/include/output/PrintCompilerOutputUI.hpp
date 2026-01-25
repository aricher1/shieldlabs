#pragma once

#include "calc/CompilerOutput.hpp"
#include "ui/UiLog.hpp"



namespace output {

    void print_to_ui(const calc::CompilerOutput& out, ui::UiLog& log);

} // end namespace output