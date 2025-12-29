#pragma once
#include <vector>


namespace calc {

    struct CalcPoint {
        double x_cm;
        double y_cm;
    };

    struct CalcLayer {
        int material_id;
        double thickness_cm;
    };

    enum class OpeningType {
        Open,
        Door,
        Window
    };

    struct CalcOpening {
        double t0;
        double t1;
        OpeningType type;
    };

    struct CalcWall {
        CalcPoint a;
        CalcPoint b;
        double length_cm;
        std::vector<CalcLayer> layers;
        std::vector<CalcOpening> openings;
    };

    struct CalcSource {
        CalcPoint position;
    };

    struct CalcDosePoint {
        CalcPoint position;
    };

    struct CalcScene {
        std::vector<CalcWall> walls;
        std::vector<CalcSource> sources;
        std::vector<CalcDosePoint> dose_points;
    };

} // end namespace calc