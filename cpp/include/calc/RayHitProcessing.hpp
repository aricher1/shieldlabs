#pragma once
#include "RayHitRecord.hpp"
#include <vector>


namespace calc {

// sort hits in-place by distance from the source
// sorts in ascending order
void sort_hits_by_distance(std::vector<RayHitRecord>& hits);

} // end namespace calc