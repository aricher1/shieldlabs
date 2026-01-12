#pragma once
#include "calc/CompilerOutput.hpp"
#include <string>


namespace output {
    
    bool export_compiler_output_csv(const calc::CompilerOutput& out, const std::string& filepath);

} // end namespace output