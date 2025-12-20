#pragma once
#include "Point.hpp"
#include <string>



enum class PointType {
    Source,
    Dose
};


struct PointEntity {
    Point position;     // cm, snapped in position
    PointType type;     // source or dose point
    std::string label;  // empty for now
};