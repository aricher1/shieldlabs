#pragma once

#include "Point.hpp"


struct Wall {
    Point a;
    Point b;
    double thickness_cm;
    int material_id;
};