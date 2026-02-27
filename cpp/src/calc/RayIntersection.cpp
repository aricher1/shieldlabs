// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include "calc/RayIntersection.hpp"


namespace calc {
    
    bool intersect_ray_with_segment ( double rx0, double ry0, double rx1, double ry1, double wx0, double wy0, double wx1, double wy1, RayHit& hit) {
        
        const double rdx = rx1 - rx0;
        const double rdy = ry1 - ry0;
        const double wdx = wx1 - wx0;
        const double wdy = wy1 - wy0;

        const double denom = rdx * wdy - rdy * wdx;
        if (std::abs(denom) < 1e-12) { return false; } // parallel or collinear
        
        const double dx = wx0 - rx0;
        const double dy = wy0 - ry0;
        const double t = (dx * wdy - dy * wdx) / denom; // along ray
        const double u = (dx * rdy - dy * rdx) / denom; // along wall

        // range check: [0,1]
        if (t < 0.0 || t > 1.0) { return false; }
        if (u < 0.0 || u > 1.0) { return false; }
    
        hit.t_ray = t;
        hit.t_wall = u;
        return true;
    }

} // end namespace calc