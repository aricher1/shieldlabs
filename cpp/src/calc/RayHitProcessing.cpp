// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include <algorithm>

#include "calc/RayHitProcessing.hpp"


namespace calc {

    void sort_hits_by_distance(std::vector<RayHitRecord>& hits) {
        std::sort(hits.begin(), hits.end(), [](const RayHitRecord& a, const RayHitRecord& b) {
                return a.distance_cm < b.distance_cm;
            }
        );
    }

} // end namespace calc