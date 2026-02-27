// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <vector>

#include "Point.hpp"



namespace geom {

    struct WallLayer {
        int material_id;
        double thickness_cm;
    };


    struct Wall {
        Point a;
        Point b;
        std::vector<WallLayer> layers;
        double length_cm;
    };

} // end namespace geom