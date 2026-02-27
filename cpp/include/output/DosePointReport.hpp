// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <vector>
#include <string>



namespace output {

    struct SourceDoseRow {
        int source_index;
        std::string source_label;
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
        std::string dose_label;
        std::vector<SourceDoseRow> rows;

        double dose_limit_uSv;
        double total_effective_dose_uSv;
        double total_annual_dose_uSv;
    };

} // end namespace output
