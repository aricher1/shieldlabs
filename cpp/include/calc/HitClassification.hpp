// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include "CalcScene.hpp"


namespace calc {

    // Enumerate through all different possible wall hit types
    enum class HitKind {
        SolidWall
    };

    // Helper to classify the wall hit once hit
    HitKind classify_wall_hit(const CalcWall& wall, double t_wall);

} // end namespace calc