#include "calc/HitClassification.hpp"


namespace calc {

    HitKind classify_wall_hit(const CalcWall& wall, double t_wall) {
        for (const auto& o : wall.openings) {
            if (t_wall >= o.t0 && t_wall <= o.t1) {
                switch (o.type) {
                    case OpeningType::Open: return HitKind::Open;
                    case OpeningType::Door: return HitKind::Door;
                    case OpeningType::Window: return HitKind::Window;
                }
            }
        }
        return HitKind::SolidWall;
    }

} // end namespace calc