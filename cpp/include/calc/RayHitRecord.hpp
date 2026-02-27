// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "HitClassification.hpp"


namespace calc {

    struct RayHitRecord {
        int wall_index;         // which wall was hit
        double t_ray;           // parametric [0,1] along source to dose
        double distance_cm;     // distance from source to hit
        HitKind kind;           // kind: SolidWall
    };

} // end namespace calc