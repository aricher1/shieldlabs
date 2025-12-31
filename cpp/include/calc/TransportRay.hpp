#pragma once
#include "CalcScene.hpp"
#include <vector>


namespace calc {

    struct TransportSegment {
        int material_id;
        double path_length_cm;
    };

    struct TransportRay {
        int source_index;
        int dose_index;
        double geometric_distance_cm;
        std::vector<TransportSegment> segments;
    };

    TransportRay build_transport_ray(const CalcScene& scene, int source_index, int dose_index);

} // end namespace calc