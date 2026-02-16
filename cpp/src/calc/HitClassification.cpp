#include "calc/HitClassification.hpp"



namespace calc {

    HitKind classify_wall_hit(const CalcWall& wall, double t_wall) {
        return HitKind::SolidWall;
    }

} // end namespace calc