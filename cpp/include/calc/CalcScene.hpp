#pragma once
#include <vector>
#include <string>


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
        std::string label;
        double num_patients;
        double activity_per_patient_MBq;
        double uptake_time_hours;
        bool apply_patient_attenuation;
        float patient_attenuation_percent;
        bool apply_radioactive_decay;
    };

    struct CalcDosePoint {
        CalcPoint position;
        std::string label;
        double occupancy;
        double dose_limit_uSv;
    };

    struct CalcScene {
        std::vector<CalcWall> walls;
        std::vector<CalcSource> sources;
        std::vector<CalcDosePoint> dose_points;
    };

} // end namespace calc