// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <vector>

#include "CalcScene.hpp"



namespace calc {

    // Material traversal for a ray segment
    struct TransportSegment {
        int wall_index;
        int material_id;
        double path_length_cm;
    };

    // Geometric ray with material traversal data
    struct TransportRay {
        int source_index;
        int dose_index;
        double geometric_distance_cm;
        std::vector<TransportSegment> segments;
    };

    // Helper to build transport ray
    TransportRay build_transport_ray(const CalcScene& scene, int source_index, int dose_index);

} // end namespace calc