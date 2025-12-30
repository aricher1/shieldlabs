#pragma once
#include <vector>
#include "CalcScene.hpp"
#include "RayHitRecord.hpp"


namespace calc {

    // collects all wall hits for one source -> dose ray
    std::vector<RayHitRecord> trace_ray(const CalcScene& scene, int source_index, int dose_index);

} // end namespace calc