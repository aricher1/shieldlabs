#pragma once
#include <vector>

namespace output {

    struct SourceDoseRow {
        int source_index;
        double integration_time_h;
        double distance_cm;
        double occupancy;
        double lead_cm;
        double concrete_cm;
        double steel_cm;
        double wall_attenuation;
        double effective_dose_uSv;
        double annual_dose_uSv;
    };

    struct DosePointReport {
        int dose_index;

        std::vector<SourceDoseRow> rows;

        double total_effective_dose_uSv;
        double total_annual_dose_uSv;
    };

} // end namespace output
