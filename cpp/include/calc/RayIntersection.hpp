#pragma once
#include <cmath>


namespace calc {

    struct RayHit {
        double t_ray;   // [0,1] along source -> dose
        double t_wall;  // [0,1] along wall a -> b
    };

    // intersects segment Ray0 -> Ray1 with segment Wall0 -> Wall1
    // will return true iff they intersect within both segments
    bool intersect_ray_with_segment (
        double rx0, double ry0,
        double rx1, double ry1,
        double wx0, double wy0,
        double wx1, double wy1,
        RayHit& hit
    );

} // end namespace calc