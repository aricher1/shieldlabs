// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


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

    struct CalcWall {                               // Wall info for calculations
        CalcPoint a;
        CalcPoint b;
        double length_cm;
        std::vector<CalcLayer> layers;
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