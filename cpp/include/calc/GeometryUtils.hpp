// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <cmath>


namespace calc {

    // Converts a parametric ray value t in [0,1] to a physical distance (in cm)
    inline double ray_distance(double sx, double sy, double dx, double dy, double t_ray) {
        
        const double L = std::hypot(dx - sx, dy - sy);
        
        return t_ray * L;
    }

} // end namespace calc
