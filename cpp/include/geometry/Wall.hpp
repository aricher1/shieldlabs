#pragma once

#include "Point.hpp"
#include <vector>



enum class OpeningType {
    Open,       // open segment along wall
    Door,       // door in wall
    Window      // window in wall
};


struct WallOpening {
    double center_t;        // where the user clicked on wall
    double length_cm;       // user-defined length
    OpeningType type;       // type: open, door, window
};


struct Wall {
    Point a;
    Point b;
    double thickness_cm;
    int material_id;
    double length_cm;

    std::vector<WallOpening> openings;
};