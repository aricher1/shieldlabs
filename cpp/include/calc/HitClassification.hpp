#pragma once
#include "CalcScene.hpp"


namespace calc {

    enum class HitKind {
        SolidWall,
        Open,
        Door,
        Window
    };

    HitKind classify_wall_hit(const CalcWall& wall, double t_wall);

} // end namespace calc