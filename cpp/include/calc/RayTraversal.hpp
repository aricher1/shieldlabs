#pragma once

#include <vector>

#include "CalcScene.hpp"
#include "RayHitRecord.hpp"



namespace calc {

    // Traces a ray from a source to a dose point and records all wall hits
    std::vector<RayHitRecord> trace_ray(const CalcScene& scene, int source_index, int dose_index);

} // end namespace calc