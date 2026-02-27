// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <string>

#include "calc/CompilerOutput.hpp"



namespace output {
    
    bool export_compiler_output_csv(const calc::CompilerOutput& out, const std::string& filepath);

} // end namespace output