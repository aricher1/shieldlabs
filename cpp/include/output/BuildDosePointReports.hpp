#pragma once
#include "calc/CompilerOutput.hpp"
#include "DosePointReport.hpp"


namespace output {

    std::vector<DosePointReport> build_dose_point_reports(const calc::CompilerOutput& out);

} // end namespace output