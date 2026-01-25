#pragma once

#include <vector>
#include <string>


namespace calc {

    struct CalcPoint {                              // Basic geometry (in centimetres)
        double x_cm;
        double y_cm;
    };

    struct CalcLayer {                              // Wall composition
        int material_id;
        double thickness_cm;
    };

    enum class OpeningType {                        // Openings on wall types
        Open,
        Door,
        Window
    };

    struct CalcOpening {                            // Openings geometry + type
        double t0;                                  // start along wall [0,1]
        double t1;                                  // end along wall [0,1]
        OpeningType type;
    };

    struct CalcWall {                               // Wall info for calculations
        CalcPoint a;
        CalcPoint b;
        double length_cm;
        std::vector<CalcLayer> layers;
        std::vector<CalcOpening> openings;
    };

    struct CalcSource {                             // Source info for calculations
        CalcPoint position;
        std::string label;
        double num_patients;
        double activity_per_patient_MBq;
        double uptake_time_hours;
        bool apply_patient_attenuation;
        float patient_attenuation_percent;
        bool apply_radioactive_decay;
    };

    struct CalcDosePoint {                          // Dose info for calculations
        CalcPoint position;
        std::string label;
        double occupancy;
        double dose_limit_uSv;
    };

    struct CalcScene {                              // Final calculation inputs
        std::string isotope_key;
        std::vector<CalcWall> walls;
        std::vector<CalcSource> sources;
        std::vector<CalcDosePoint> dose_points;
    };

} // end namespace calc