// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "CalcScene.hpp"
#include "CompilerOutput.hpp"
#include "isotopes/IsotopeRegistry.hpp"



namespace calc {

    // Evaluates a single source–dose pair through the full physics pipeline
    CompilerRayOutput evaluate_ray_pipeline(
        const CalcScene& scene,
        int source_index,
        int dose_index,
        const isotope::IsotopeDef& isotope
    );

} // end namespace calc