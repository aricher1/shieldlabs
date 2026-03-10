// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <vector>

#include "calc/CompilerOutput.hpp"
#include "DosePointReport.hpp"



namespace output {

    std::vector<DosePointReport> build_dose_point_reports(const calc::CompilerOutput& out);

} // end namespace output