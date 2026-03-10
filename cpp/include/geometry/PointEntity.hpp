// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <string>
#include <optional>

#include "Point.hpp"



namespace geom {

    enum class PointType {
        Source,
        Dose
    };

    // Source parameters
    struct SourceData {
        float num_patients;                  // number of patients per week
        float activity_per_patient_MBq;      // MBq per patient
        float uptake_time_hours;             // hours
        bool apply_patient_attenuation;      // user-defined -> yes/no
        float patient_attenuation_percent;   // user-defined [0.0, 1.0] = 0-100%
        bool apply_radioactive_decay;        // user-defined -> yes/no
    };

    // Dose parameters
    struct DoseData {
        float occupancy;                     // occupancy factor (0–1)
        std::string occupancy_type;          // e.g. controlled / uncontrolled
        float dose_limit_uSv;                // dose limit (µSv/week or similar)
    };

    // Unified point entity (source or dose)
    struct PointEntity {
        Point position;                      // cm, snapped
        PointType type;                      // Source or Dose
        std::string label;                   // optional label
        std::optional<SourceData> source;    // valid iff type == Source
        std::optional<DoseData> dose;        // valid iff type == Dose
    };

} // end namespace geom