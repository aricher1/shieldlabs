// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#include "calc/HitClassification.hpp"


namespace calc {

    HitKind classify_wall_hit([[maybe_unused]]const CalcWall& wall, [[maybe_unused]]double t_wall) {
        return HitKind::SolidWall;
    }

} // end namespace calc