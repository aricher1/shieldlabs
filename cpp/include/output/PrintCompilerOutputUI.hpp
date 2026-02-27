// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "calc/CompilerOutput.hpp"
#include "ui/UiLog.hpp"



namespace output {

    void print_to_ui(const calc::CompilerOutput& out, ui::UiLog& log);

} // end namespace output