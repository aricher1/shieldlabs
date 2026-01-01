#pragma once
#include "Point.hpp"
#include <string>
#include <optional>

enum class PointType {
    Source,
    Dose
};

// ---------------- Source-specific data ---------------- //
struct SourceData {
    float num_patients;                  // number of patients per week
    float activity_per_patient_MBq;      // MBq per patient
    float uptake_time_hours;             // hours
    bool apply_patient_attenuation;      // user-defined -> yes/no
    bool apply_radioactive_decay;        // user-defined -> yes/no
};

// ---------------- Dose-specific data ---------------- //
struct DoseData {
    float occupancy;                     // occupancy factor (0–1)
    std::string occupancy_type;          // e.g. controlled / uncontrolled
    float dose_limit_uSv;                // dose limit (µSv/week or similar)
};

// ---------------- Unified entity ---------------- //
struct PointEntity {
    Point position;                      // cm, snapped
    PointType type;                      // Source or Dose
    std::string label;                   // optional label
    std::optional<SourceData> source;    // valid iff type == Source
    std::optional<DoseData> dose;        // valid iff type == Dose
};
