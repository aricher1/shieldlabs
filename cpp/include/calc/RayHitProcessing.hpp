// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <vector>

#include "RayHitRecord.hpp"



namespace calc {

    // Sorts ray hits in ascending order by distance from the source
    void sort_hits_by_distance(std::vector<RayHitRecord>& hits);

} // end namespace calc