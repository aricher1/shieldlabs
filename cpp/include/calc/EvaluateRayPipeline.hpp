#pragma once
#include "CalcScene.hpp"
#include "CompilerOutput.hpp"
#include "isotopes/IsotopeRegistry.hpp"

/*
    Executes the full physics pipeline for ONE source → ONE dose:

    - builds transport ray
    - evaluates instantaneous dose
    - integrates dose over time (decay)
    - returns fully populated CompilerRayOutput

    This function contains NO aggregation logic.
*/

namespace calc {

CompilerRayOutput evaluate_ray_pipeline(
    const CalcScene& scene,
    int source_index,
    int dose_index,
    const IsotopeDef& isotope
);

} // end namespace calc