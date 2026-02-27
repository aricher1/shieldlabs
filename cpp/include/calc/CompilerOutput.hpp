// ============================================================================================
// ShieldLabs
// Copyright (c) 2026 Aidan Richer
// Licensed under the MIT License. See LICENSE file for details.
// ============================================================================================


#pragma once

#include <vector>
#include <string>

#include "TransportRay.hpp"
#include "EvaluateSingleRay.hpp"
#include "IntegrateDose.hpp"



namespace calc {

    struct CompilerRayOutput {                                      // Result of a single ray
        int source_index;
        int dose_index;
        TransportRay ray;
        SingleRayDoseResult dose;
        IntegratedDoseResult integrated;
        std::string source_label;
        std::string dose_label;
    };

    struct DosePointTotal {                                         // Aggregated dose results for a single dose point
        int dose_index;
        std::string dose_name;
        double dose_limit_uSv;
        double integrated_dose_uSv = 0.0;
        double average_rate_uSv_h = 0.0;
        double occupancy = 1.0;
        double effective_dose_uSv = 0.0;
        double annual_dose_uSv = 0.0;
    };

    struct CompilerOutput {                                         // Full calculation output
        std::string isotope_key;
        std::vector<CompilerRayOutput> rays;
        std::vector<DosePointTotal> dose_totals;
    };
        
    CompilerOutput build_compiler_output(const CalcScene& scene);

} // end namespace calc