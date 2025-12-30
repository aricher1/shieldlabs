#include "calc/RayHitProcessing.hpp"
#include <algorithm>


namespace calc {

void sort_hits_by_distance(std::vector<RayHitRecord>& hits) {
    std::sort(
        hits.begin(), hits.end(), 
        [](const RayHitRecord& a, const RayHitRecord& b) {
            return a.distance_cm < b.distance_cm;
        }
    );
}

} // end namespace calc