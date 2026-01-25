#pragma once

#include "CalcScene.hpp"



namespace calc {

    // Enumerate through all different possible wall hit types
    enum class HitKind {
        SolidWall,
        Open,
        Door,
        Window
    };

    // Helper to classify the wall hit once hit
    HitKind classify_wall_hit(const CalcWall& wall, double t_wall);

} // end namespace calc