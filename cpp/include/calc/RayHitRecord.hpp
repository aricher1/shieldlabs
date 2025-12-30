#pragma once
#include "HitClassification.hpp"


namespace calc {

    struct RayHitRecord {
        int wall_index;         // which wall was hit
        double t_ray;           // parametric [0,1] along source -> dose
        double distance_cm;     // distance from source to hit
        HitKind kind;           // kind: SolidWall, Open, Door, Window
    };

} // end namespace calc