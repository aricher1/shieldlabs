#pragma once
#include <cmath>

namespace calc {

    // distance along a ray segment from source to a hit point
    // expressed as parametric t_ray in [0,1]
    inline double ray_distance(double sx, double sy, double dx, double dy, double t_ray) {
        const double L = std::hypot(dx - sx, dy - sy);
        return t_ray * L;
    }

} // end namespace calc
